* Somehow check if a neovim client has disconnected before we get to know this when we want to tell it to change theme (ping-pong?) (send some dummy commands like `nvim_get_vvar()` and read its results?)
* Singleton server must be stopped when there are no neovim clients left
* Add support for IP sockets in neovim client implementation, because neovim on Windows still uses named pipes instead of `AF_UNIX` sockets ([more info](https://github.com/neovim/neovim/issues/11363))
* Remove singleton server socket file when the program is about to close
* Check `$XDG_SESSION_TYPE`. If it is equal to `tty`, then do not start anything (should probably be implemented on Lua side)
* Make use of command-line arguments
* (optional, because Win10 has added support for `AF_UNIX` sockets since like 2018) Add support for IP sockets in singleton client/server implementation
* Each folder must have its own `meson.build` file with its own list of dependencies (e.g. only `monitor::application` must have the access to the `siga::dark_notify`)
