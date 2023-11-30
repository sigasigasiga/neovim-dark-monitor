#include "monitor/service/neovim.hpp"

namespace monitor::service {

neovim_t::neovim_t(boost::asio::any_io_executor exec,
                   boost::asio::generic::stream_protocol::socket current_client)
    : exec_{std::move(exec)} {
  clients_.make_job(std::move(current_client));
}

void neovim_t::on_client_msg(msgpack::object_handle handle) {
  auto socket_path = handle.get().as<std::string>();
  boost::asio::local::stream_protocol::endpoint endpoint{
      std::move(socket_path)};
  auto socket_ptr =
      std::make_unique<boost::asio::local::stream_protocol::socket>(exec_);
  auto &socket_ref = *socket_ptr;

  auto cb = [this, socket_ptr = std::move(socket_ptr)](
                const boost::system::error_code &ec) {
    if (ec) {
      spdlog::error("Cannot connect to the socket: {} ({})", ec.message(),
                    ec.value());
    } else {
      clients_.make_job(std::move(*socket_ptr));
    }
  };

  socket_ref.async_connect(endpoint, wrap(std::move(cb)));
}

} // namespace monitor::service
