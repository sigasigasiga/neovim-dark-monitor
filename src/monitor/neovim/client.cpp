#include "monitor/neovim/client.hpp"

#include "monitor/util/error.hpp"

namespace monitor::neovim {

client_t::client_t(std::unique_ptr<job_t::delegate_t> job_delegate,
                   boost::asio::generic::stream_protocol::socket socket)
    : job_t{std::move(job_delegate)}, socket_{std::move(socket)} {}

void client_t::process_queue() {
  assert(!buffer_queue_.empty());
  const auto &current_buffer = buffer_queue_.front();
  boost::asio::async_write(
      socket_,
      boost::asio::buffer(current_buffer.data(), current_buffer.size()),
      wrap(std::bind_front(&client_t::handle_write, this)));
}

void client_t::handle_write(const boost::system::error_code &ec,
                            std::size_t bytes) {
  buffer_queue_.pop_front();

  if (ec) {
    if (util::is_disconnected(ec)) {
      spdlog::info("The client has disconnected");
    } else {
      spdlog::error("Cannot write to the nvim socket: {} ({})", ec.message(),
                    ec.value());
    }

    return job_finished();
  }

  spdlog::debug("{} bytes has written", bytes);
  if (!buffer_queue_.empty()) {
    process_queue();
  }
}

} // namespace monitor::neovim
