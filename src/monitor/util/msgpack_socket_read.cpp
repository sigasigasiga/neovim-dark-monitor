#include "monitor/util/msgpack_socket_read.hpp"

namespace monitor::util {

msgpack_socket_read_t::msgpack_socket_read_t(
    delegate_t &delegate, boost::asio::generic::stream_protocol::socket socket)
    : delegate_{delegate}, socket_{std::move(socket)} {
  async_read();
}

void msgpack_socket_read_t::async_read() {
  constexpr std::size_t window_size = 128;
  unpacker_.reserve_buffer(window_size);
  socket_.async_read_some(
      boost::asio::buffer(unpacker_.buffer(), window_size),
      wrap(std::bind_front(&msgpack_socket_read_t::handle_read, this)));
}

void msgpack_socket_read_t::handle_read(const boost::system::error_code &ec,
                                        std::size_t bytes) {
  if (ec) {
    return delegate_.on_error(ec);
  }

  unpacker_.buffer_consumed(bytes);
  msgpack::object_handle handle;
  while (unpacker_.next(handle)) {
    delegate_.on_message_received(std::move(handle));
  }

  async_read();
}

} // namespace monitor::util
