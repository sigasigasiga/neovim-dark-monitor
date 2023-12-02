#pragma once

#include "monitor/singleton/server.hpp"
#include "monitor/util/functional.hpp"
#include "monitor/util/service.hpp"

namespace monitor::service {

class singleton_t : public util::service_t {
public:
  singleton_t(boost::asio::any_io_executor exec,
              boost::asio::local::stream_protocol::endpoint ep,
              singleton::server_t::msg_handler_t &msg_handler);

private: // util::service_t
  void reload() final;

private:
  util::first_flag_t first_;
  singleton::server_t server_;
};

} // namespace monitor::service
