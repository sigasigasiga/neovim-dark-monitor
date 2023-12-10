#include "monitor/neovim/client.hpp"

#include "monitor/util/error.hpp"

namespace monitor::neovim {

client_t::client_t(std::unique_ptr<job_t::delegate_t> job_delegate,
                   boost::asio::generic::stream_protocol::socket socket)
    : job_t{std::move(job_delegate)}, socket_{std::move(socket)},
      reader_{*this, socket_}, writer_{*this, socket_} {}

// util::asio::msgpack_socket_read_t<>::delegate_t,
void client_t::on_message_received(msgpack::object_handle handle) {
  // TODO: forward this information to the callback
  std::ostringstream oss;
  oss << handle.get();
  spdlog::info("got message {}", oss.str());
}

void client_t::on_read_error(const boost::system::error_code &ec) {
  assert(ec);

  if (util::is_disconnected(ec)) {
    spdlog::info("The client has disconnected");
  } else {
    spdlog::error("Cannot read from the nvim socket: {} ({})", ec.message(),
                  ec.value());
  }

  job_finished();
}

// util::asio::msgpack_socket_write_t<>::delegate_t
void client_t::on_write_error(const boost::system::error_code &ec) {
  assert(ec);

  if (util::is_disconnected(ec)) {
    spdlog::info("The client has disconnected");
  } else {
    spdlog::error("Cannot write to the nvim socket: {} ({})", ec.message(),
                  ec.value());
  }

  return job_finished();
}

} // namespace monitor::neovim
