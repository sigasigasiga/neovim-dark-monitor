#pragma once

#include "monitor/util/callback_wrapper.hpp"

namespace monitor::util::asio {

class msgpack_socket_read_t : private callback_wrapper_t {
public:
  class delegate_t {
  public:
    virtual ~delegate_t() = default;
    virtual void on_message_received(msgpack::object_handle handle) = 0;
    virtual void on_error(const boost::system::error_code &ec) = 0;
  };

public:
  msgpack_socket_read_t(delegate_t &delegate,
                        boost::asio::generic::stream_protocol::socket socket);

private:
  void async_read();
  void handle_read(const boost::system::error_code &ec, std::size_t bytes);

private:
  delegate_t &delegate_;
  boost::asio::generic::stream_protocol::socket socket_;
  msgpack::unpacker unpacker_;
};

} // namespace monitor::util::asio
