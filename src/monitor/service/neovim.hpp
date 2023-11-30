#pragma once

#include "monitor/neovim/client.hpp"
#include "monitor/singleton/server.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/service.hpp"

namespace monitor::service {

class neovim_t : public util::service_t,
                 public singleton::server_t::msg_handler_t,
                 private util::callback_wrapper_t {
public:
  neovim_t(boost::asio::any_io_executor exec,
           boost::asio::generic::stream_protocol::socket current_client);

public:
  auto clients() const { return clients_.range(); }

private: // singleton::server_t::msg_handler_t
  void on_client_msg(msgpack::object_handle handle) final;

private:
  boost::asio::any_io_executor exec_;
  util::job_storage_t<neovim::client_t> clients_;
};

} // namespace monitor::service
