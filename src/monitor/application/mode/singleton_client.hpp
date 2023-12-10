#pragma once

#include "monitor/application/application.hpp"
#include "monitor/singleton/client.hpp"

namespace monitor::application::mode {

class singleton_client_t : public application_t::mode_t,
                           public singleton::client_t::state_dumper_t,
                           private util::scoped_t {
public:
  singleton_client_t(const std::string &singleton_socket,
                     const std::string &nvim_socket);

public:
  bool try_client_mode();

private: // application_t::mode_t
  void run() final;

private: // singleton::client_t::state_dumper_t
  msgpack::sbuffer dump_state() final;

private:
  boost::asio::io_context io_;

  // TODO: add support for generic sockets
  std::variant<boost::asio::local::stream_protocol::endpoint,
               singleton::client_t>
      state_;

  const std::string &nvim_socket_;
};

} // namespace monitor::application::mode
