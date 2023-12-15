#pragma once

#include "monitor/neovim/proto/message.hpp"
#include "monitor/util/asio/msgpack_socket_read.hpp"
#include "monitor/util/asio/msgpack_socket_write.hpp"
#include "monitor/util/job.hpp"

namespace monitor::neovim {

class client_t : public util::job_t,
                 public util::asio::msgpack_socket_read_t<>::delegate_t,
                 public util::asio::msgpack_socket_write_t<>::delegate_t {
public:
  client_t(std::unique_ptr<job_t::delegate_t> job_delegate,
           boost::asio::generic::stream_protocol::socket socket);

public:
  template <proto::rpc_method Method> void send_request(const Method &method);

private: // util::asio::msgpack_socket_read_t<>::delegate_t,
  void on_message_received(msgpack::object_handle handle) final;
  void on_read_error(const boost::system::error_code &ec) final;

private: // util::asio::msgpack_socket_write_t<>::delegate_t
  void on_write_error(const boost::system::error_code &ec) final;

private:
  boost::asio::generic::stream_protocol::socket socket_;
  util::asio::msgpack_socket_read_t<> reader_;
  util::asio::msgpack_socket_write_t<> writer_;

  std::int64_t message_id_ = 0;
};

template <proto::rpc_method Method>
void client_t::send_request(const Method &method) {
  msgpack::sbuffer buf;
  msgpack::packer packer{buf};

  proto::message_t<Method> msg{
      .msg_type = proto::msg_type_t::request,
      .message_id = message_id_++,
      .method_name = static_cast<std::string>(Method::method_name),
      .data = method};

  packer.pack(msg);
  writer_.write(std::move(buf));
}

} // namespace monitor::neovim
