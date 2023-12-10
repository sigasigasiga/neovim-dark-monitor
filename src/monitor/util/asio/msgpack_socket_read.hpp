#pragma once

#include "monitor/util/callback_wrapper.hpp"

namespace monitor::util::asio {

template <typename Socket = boost::asio::generic::stream_protocol::socket>
class msgpack_socket_read_t : private callback_wrapper_t {
public:
  class delegate_t {
  public:
    virtual ~delegate_t() = default;
    virtual void on_message_received(msgpack::object_handle handle) = 0;
    virtual void on_read_error(const boost::system::error_code &ec) = 0;
  };

public:
  msgpack_socket_read_t(delegate_t &delegate, Socket &socket);

private:
  void async_read();
  void handle_read(const boost::system::error_code &ec, std::size_t bytes);

private:
  delegate_t &delegate_;
  Socket &socket_;
  msgpack::unpacker unpacker_;
};

template <typename Socket>
msgpack_socket_read_t<Socket>::msgpack_socket_read_t(delegate_t &delegate,
                                                     Socket &socket)
    : delegate_{delegate}, socket_{socket} {
  async_read();
}

template <typename Socket> void msgpack_socket_read_t<Socket>::async_read() {
  constexpr std::size_t window_size = 128;
  unpacker_.reserve_buffer(window_size);
  socket_.async_read_some(
      boost::asio::buffer(unpacker_.buffer(), window_size),
      wrap(std::bind_front(&msgpack_socket_read_t::handle_read, this)));
}

template <typename Socket>
void msgpack_socket_read_t<Socket>::handle_read(
    const boost::system::error_code &ec, std::size_t bytes) {
  if (ec) {
    return delegate_.on_read_error(ec);
  }

  unpacker_.buffer_consumed(bytes);
  msgpack::object_handle handle;
  while (unpacker_.next(handle)) {
    delegate_.on_message_received(std::move(handle));
  }

  async_read();
}

} // namespace monitor::util::asio
