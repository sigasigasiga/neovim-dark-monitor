#include "monitor/singleton/client.hpp"

namespace monitor::singleton {

namespace {

void handle_write(const boost::system::error_code &ec,
                  std::size_t /* bytes */) {
  if (ec) {
    throw boost::system::system_error{ec, "singleton client write"};
  }
}

} // anonymous namespace

client_t::client_t(state_dumper_t &state_dumper,
                   boost::asio::generic::stream_protocol::socket socket)
    : state_dumper_{state_dumper}, socket_{std::move(socket)} {}

void client_t::start() {
  buffer_ = state_dumper_.dump_state();
  boost::asio::async_write(socket_,
                           boost::asio::buffer(buffer_.data(), buffer_.size()),
                           handle_write);
}

} // namespace monitor::singleton
