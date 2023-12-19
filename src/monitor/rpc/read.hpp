#pragma once

#include "monitor/util/callback_wrapper.hpp"

namespace monitor::rpc {

template <typename Socket = boost::asio::generic::stream_protocol::socket>
class read_t : private util::callback_wrapper_t {
public:
  class delegate_t {
  public:
    virtual ~delegate_t() = default;
    virtual void on_message_received(msgpack::object_handle handle) = 0;
    virtual void on_read_error(const boost::system::error_code &ec) = 0;
  };

public:
  read_t(delegate_t &delegate, Socket &socket);

private:
  void async_read();
  void handle_read(const boost::system::error_code &ec, std::size_t bytes);

private:
  delegate_t &delegate_;
  Socket &socket_;
  msgpack::unpacker unpacker_;
};

template <typename Socket>
read_t<Socket>::read_t(delegate_t &delegate, Socket &socket)
    : delegate_{delegate}, socket_{socket} {
  async_read();
}

template <typename Socket> void read_t<Socket>::async_read() {
  constexpr std::size_t window_size = 128;
  unpacker_.reserve_buffer(window_size);
  socket_.async_read_some(boost::asio::buffer(unpacker_.buffer(), window_size),
                          wrap(std::bind_front(&read_t::handle_read, this)));
}

template <typename Socket>
void read_t<Socket>::handle_read(const boost::system::error_code &ec,
                                 std::size_t bytes) {
  if (ec) {
    return delegate_.on_read_error(ec);
  }

  unpacker_.buffer_consumed(bytes);
  msgpack::object_handle handle;
  while (unpacker_.next(handle)) {
    std::ostringstream oss;
    oss << handle.get();
    spdlog::trace("Got message: {}", oss.str());
    delegate_.on_message_received(std::move(handle));
  }

  async_read();
}

} // namespace monitor::rpc
