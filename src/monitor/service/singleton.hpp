#pragma once

#include "monitor/singleton/server.hpp"
#include "monitor/util/functional.hpp"
#include "monitor/util/service.hpp"

namespace monitor::service {

class singleton_t : public util::service_t,
                    public singleton::server_t::msg_handler_t {
public:
  singleton_t(boost::asio::any_io_executor exec,
              boost::asio::local::stream_protocol::endpoint ep);

private: // util::service_t
  void reload() final;

private: // singleton::server_t::msg_handler_t
  void on_client_msg(msgpack::object_handle handle) final;

private:
  util::first_flag_t first_;
  singleton::server_t server_;
};

} // namespace monitor::service
