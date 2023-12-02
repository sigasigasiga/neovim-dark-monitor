* New neovim clients must receive their new theme whenever they are connected to the singleton server
* Singleton server must be stopped when there are no neovim clients left
* `siga::dark_notify::dark_notify_t::run` must be executed on the main thread on macOS
* Sometimes there is some strange unhandled exception (something about `std::thread::join`) when a singleton client connects to the singleton server
* Remove singleton server socket file when the program is about to close
* Add support for IP sockets in neovim client implementation, because neovim on Windows still uses named pipes instead of `AF_UNIX` sockets ([more info](https://github.com/neovim/neovim/issues/11363))
* (optional, because Win10 has added support for `AF_UNIX` sockets since like 2018) Add support for IP sockets in singleton client/server implementation
* Make use of command-line arguments
* Somehow check if a neovim client has disconnected before we get to know this when we want to tell it to change theme (ping-pong?)
