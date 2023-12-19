#pragma once

#include "monitor/util/callback_wrapper.hpp"

namespace monitor::rpc {

template <typename Socket = boost::asio::generic::stream_protocol::socket>
class write_t : private util::callback_wrapper_t {
public:
  class delegate_t {
  public:
    virtual ~delegate_t() = default;
    virtual void on_write_error(const boost::system::error_code &ec) = 0;
  };

public:
  write_t(delegate_t &delegate, Socket &socket);

public:
  void write(msgpack::object object);

private:
  void process_queue();
  void handle_write(const boost::system::error_code &ec, std::size_t bytes);

private:
  delegate_t &delegate_;
  Socket &socket_;
  std::deque<msgpack::sbuffer> buffer_queue_;
};

template <typename Socket>
write_t<Socket>::write_t(delegate_t &delegate, Socket &socket)
    : delegate_{delegate}, socket_{socket} {}

template <typename Socket> void write_t<Socket>::write(msgpack::object object) {
  msgpack::sbuffer buf;
  msgpack::packer packer{buf};
  packer.pack(object);

  std::ostringstream oss;
  oss << object;
  spdlog::trace("Writing {}", oss.str());

  buffer_queue_.push_back(std::move(buf));
  if (buffer_queue_.size() == 1) {
    boost::asio::post(socket_.get_executor(),
                      wrap(std::bind_front(&write_t::process_queue, this)));
  }
}

template <typename Socket> void write_t<Socket>::process_queue() {
  assert(!buffer_queue_.empty());
  const auto &current_buffer = buffer_queue_.front();
  boost::asio::async_write(
      socket_,
      boost::asio::buffer(current_buffer.data(), current_buffer.size()),
      wrap(std::bind_front(&write_t::handle_write, this)));
}

template <typename Socket>
void write_t<Socket>::handle_write(const boost::system::error_code &ec,
                                   std::size_t /* bytes */) {
  buffer_queue_.pop_front();

  if (ec) {
    delegate_.on_write_error(ec);
  } else if (!buffer_queue_.empty()) {
    process_queue();
  }
}

} // namespace monitor::rpc
