# neovim-dark-monitor

A [libdarknotify](https://github.com/sigasigasiga/libdarknotify) based neovim plugin that allows to automatically change the editor theme whenever the OS theme is changed too.

## Supported platforms

* Windows
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

It is recommended to copy the resulting executable to `~/.config/nvim/rplugin/neovim-dark-monitor`.

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
