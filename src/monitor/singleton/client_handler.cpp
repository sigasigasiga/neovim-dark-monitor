#include "monitor/singleton/client_handler.hpp"

#include "monitor/util/error.hpp"

namespace monitor::singleton {

client_handler_t::client_handler_t(
    std::unique_ptr<job_t::delegate_t> job_delegate, msg_handler_t &msg_handler,
    boost::asio::generic::stream_protocol::socket client_socket)
    : job_t{std::move(job_delegate)}, msg_handler_{msg_handler},
      client_socket_{std::move(client_socket)}, reader_{*this, client_socket_} {
}

// util::asio::msgpack_socket_read_t::delegate_t
void client_handler_t::on_message_received(msgpack::object_handle handle) {
  msg_handler_.on_client_msg(std::move(handle));
}

void client_handler_t::on_read_error(const boost::system::error_code &ec) {
  if (util::is_disconnected(ec)) {
    spdlog::info("client disconnected");
  } else {
    spdlog::error("Client disconnected with an error ({}): {}", ec.value(),
                  ec.message());
  }

  job_finished();
}

} // namespace monitor::singleton
