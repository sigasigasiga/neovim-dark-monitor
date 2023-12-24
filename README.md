# neovim-dark-monitor

A [libdarknotify](https://github.com/sigasigasiga/libdarknotify) based neovim plugin that allows to automatically change the editor theme whenever the OS theme is changed too.

## Supported platforms

* Windows 10 or newer
* macOS
* Linux/FreeBSD/other platforms that support GLib and `org.freedesktop.appearance.color-scheme`

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

### TL;DR

It is recommended to copy the resulting executable to `~/.config/nvim/rplugin/neovim-dark-monitor`.

**NOTE**: `neovim-dark-monitor` only supports sockets, but currently Neovim for Windows opens named pipe instead of `AF_UNIX`, so the call to `serverstart()` is needed before the process start. (_TODO_: add an example)

The example of the lua config file:
```lua
if os.getenv('XDG_SESSION_TYPE') == 'tty' then
    return
end

local dark_monitor_exe = vim.api.nvim_get_runtime_file('rplugin/neovim-dark-monitor', false)[1]
if type(dark_monitor_exe) == 'string' then
    -- It is better to use `vim.system` on neovim version 0.10+
    local run_cmd = string.format('call jobstart("%s", {"detach": v:true, "stdin": "null"})', dark_monitor_exe)
    vim.cmd(run_cmd)
end

local set_theme = function(theme)
    if theme == 'dark' then
        -- PLACE YOUR HANDLERS HERE
    elseif theme == 'light' then
        -- PLACE YOUR HANDLERS HERE
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

### Full manual

When connected to the `v:servername`, the `neovim-dark-monitor` first registers `DarkMonitorQuery` Lua function, then invokes `OnDarkMonitorConnected` autocommand.
Then every time the OS theme is changed, the `OnDarkMonitorThemeChange` is invoked.

## Comparing `neovim-dark-monitor` to other implementations

### [vim-lumen](https://github.com/vimpostor/vim-lumen)

Its pros:
* No need to build anything
* Works with both vim and neovim

Its cons:
* For each new vim instance it spawns another process which monitors the system theme, whereas `neovim-dark-monitor` can control all the instances within a single process
* It doesn't provide an ability to query current theme in any time
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
