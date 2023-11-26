#include "monitor/service/singleton.hpp"

#include <iostream> // TODO: remove this

namespace monitor::service {

singleton_t::singleton_t(boost::asio::any_io_executor exec,
                         boost::asio::local::stream_protocol::endpoint ep)
    : server_{*this, std::move(exec), std::move(ep)} {}

// util::service_t
void singleton_t::reload() {
  if (first_()) {
    server_.start();
  }
}

// singleton::server_t::msg_handler_t
void singleton_t::on_client_msg(msgpack::object_handle handle) {
  std::cout << handle.get() << std::endl; // TODO: remove this
}

} // namespace monitor::service
