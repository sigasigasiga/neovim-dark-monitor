# neovim-dark-monitor

A [libdarknotify](https://github.com/sigasigasiga/libdarknotify) based neovim plugin that allows to automatically change the editor theme whenever the OS theme is changed too.

## Supported platforms

* Windows 10 or newer
* macOS
* Linux/FreeBSD/other platforms that support GLib and `org.freedesktop.appearance.color-scheme`
* WSL

## Building instructions

### Requirements

* C++20 supporting compiler
* meson build system
* Compile-time dependencies (optional, as they can be installed by the meson build system automatically):
    * range-v3
    * msgpack-cxx
    * tl-expected
* Run-time dependencies:
    * Boost
    * (Linux only) GLib (including GIO and GObject)

### Compilation

```sh
git submodule update --init --recursive
meson wrap update-db
meson setup -Ddefault_library=static --buildtype=release bin
ninja -C bin
```

## Usage

### How does it work?

When connected to the `v:servername`, `neovim-dark-monitor` first registers `DarkMonitorQuery` Lua function, then invokes an `OnDarkMonitorConnected` autocommand.
Then every time the OS theme is changed, the `OnDarkMonitorThemeChange` is invoked.

### What do I need to do?

It is recommended to copy the resulting executable to `~/.config/nvim/rplugin/neovim-dark-monitor.exe` (an `.exe` extension is needed to make a single config usable for Windows and other systems).

The example of the lua config file:
```lua
local dark_monitor_exe = vim.api.nvim_get_runtime_file('rplugin/neovim-dark-monitor.exe', false)
if next(dark_monitor_exe) == nil then
    return
end

local current_os = vim.loop.os_uname()
if os.getenv('XDG_SESSION_TYPE') == 'tty' then
    return
end

-- Currently Neovim on Windows spawns a named pipe instead of a regular socket on startup,
-- but `neovim-dark-monitor` doesn't support them.
--
-- This is a subject to change on the Neovim side, because Windows has added a support
-- for `AF_UNIX` sockets since 2018 (more info: https://github.com/neovim/neovim/issues/11363 ),
-- but currently we have to work this around by spawning an IP socket manually.
--
-- Under WSL we need to run the Win32 executable to make it actually work, but Win32 applications
-- cannot connect to the UNIX sockets that are spawned in the VM, so we also have to spawn an IP socket
local is_windows_pipes = current_os.sysname:find('Windows') and vim.v.servername:find('\\\\.\\pipe') == 1
local is_wsl = current_os.release:find('WSL')
if is_windows_pipes or is_wsl then
    local server = '127.0.0.1:0' -- the port will be chosen arbitrarily

    -- When we call `serverstart`, the new server socket is added to the end of
    -- the `serverlist()` and there's no way to do that the other way.
    --
    -- At the same time, `v:servername` is always equal to the first element in
    -- the list, so unfortunately we have to stop the old server and spawn
    -- a new one just because we need it to come first in the list.
    assert(#vim.fn.serverlist() == 1, 'More than one RPC server has been started')
    vim.fn.serverstop(vim.v.servername)
    vim.fn.serverstart(server)
end

assert(#dark_monitor_exe == 1)
-- It is better to use `vim.system` on neovim version 0.10+
local job_id = vim.fn.jobstart(dark_monitor_exe, {
    detach = true,
    stdin = nil
})
assert(job_id > 0, 'Unable to start the `neovim-dark-monitor` job')

local set_theme = function(theme)
    if theme == 'dark' then
        -- PUT YOUR HANDLERS HERE
    elseif theme == 'light' then
        -- PUT YOUR HANDLERS HERE
    else
        vim.print(string.format('neovim-dark-monitor: Theme is unknown (%s)', tostring(theme)))
    end
end

local dark_monitor_group = vim.api.nvim_create_augroup('DarkMonitorConfig', { clear = true })
vim.api.nvim_create_autocmd('User', {
    group = dark_monitor_group,
    pattern = 'OnDarkMonitorConnected',
    callback = function(ev)
        set_theme(DarkMonitorQuery())
    end
})
vim.api.nvim_create_autocmd('User', {
    group = dark_monitor_group,
    pattern = 'OnDarkMonitorThemeChange',
    callback = function(ev)
        local theme = ev.data
        set_theme(theme)
    end
})
```

## Comparing `neovim-dark-monitor` to other implementations

### [vim-lumen](https://github.com/vimpostor/vim-lumen)

Its pros:
* No need to build anything
* Works with both vim and neovim

Its cons:
* For each new vim instance it spawns another process which monitors the system theme, whereas `neovim-dark-monitor` can control all the instances within a single process
* It doesn't provide an ability to query current theme at any time
* It is written in vimscript, so it should be a bit slower (but you probably won't notice that anyway though)

### [auto-dark-mode.nvim](https://github.com/f-person/auto-dark-mode.nvim)

Its pros:
* No need to build anything

Its cons:
* Spawns a new process each N seconds to query the current theme

### [dark-notify](https://github.com/cormacrelf/dark-notify)

Its cons:
* Strange Lua API
* Works only on macOS
* Parses `stdin` internally, which is slow and bodgy
* `rustc` doesn't come by default on many systems
