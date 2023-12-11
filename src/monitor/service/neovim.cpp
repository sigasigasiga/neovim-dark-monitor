#include "monitor/service/neovim.hpp"

#include "monitor/util/asio/endpoint.hpp"

namespace monitor::service {

neovim_t::neovim_t(boost::asio::any_io_executor exec,
                   boost::asio::generic::stream_protocol::socket current_client)
    : exec_{std::move(exec)} {
  clients_.make_job(std::move(current_client));
}

// singleton::server_t::msg_handler_t
void neovim_t::on_client_msg(msgpack::object_handle handle) {
  auto socket = handle.get().as<std::string>();
  auto endpoint = util::asio::make_endpoint(socket);
  if (!endpoint) {
    spdlog::error(
        "Got an invalid neovim endpoint path from the singleton client");
    return;
  }

  auto socket_ptr =
      std::make_unique<boost::asio::generic::stream_protocol::socket>(exec_);
  auto &socket_ref = *socket_ptr;

  auto cb = [this, socket_ptr = std::move(socket_ptr)](
                const boost::system::error_code &ec) {
    if (ec) {
      spdlog::error("Cannot connect to the socket: {} ({})", ec.message(),
                    ec.value());
    } else {
      auto &client = clients_.make_job(std::move(*socket_ptr));
      new_client_sig_(client);
    }
  };

  socket_ref.async_connect(*endpoint, wrap(std::move(cb)));
}

} // namespace monitor::service
