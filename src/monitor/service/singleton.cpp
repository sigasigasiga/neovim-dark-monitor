#include "monitor/service/singleton.hpp"

namespace monitor::service {

singleton_t::singleton_t(boost::asio::any_io_executor exec,
                         boost::asio::local::stream_protocol::endpoint ep,
                         singleton::server_t::msg_handler_t &msg_handler)
    : server_{msg_handler, std::move(exec), std::move(ep)} {}

// util::service_t
void singleton_t::reload() {
  if (first_()) {
    server_.start();
  }
}

} // namespace monitor::service
