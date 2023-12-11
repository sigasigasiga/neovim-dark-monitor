* Singleton server must be stopped when there are no neovim clients left
* Remove singleton server socket file when the program is about to close
* Check `$XDG_SESSION_TYPE`. If it is equal to `tty`, then do not start anything. That should probably be implemented on Lua side, but it won't hurt to also check that in the program
* Make use of command-line arguments
* (optional, because Win10 has added support for `AF_UNIX` sockets since like 2018) Add support for IP sockets in singleton client/server implementation
* Each folder must have its own `meson.build` file with its own list of dependencies (e.g. only `monitor::application` must have the access to the `siga::dark_notify`)
* Do not build `dark_notify/example.cpp` (use `get_option`?)
* configure logger
