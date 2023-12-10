#pragma once

#include "monitor/neovim/proto.hpp"
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
  template <typename... RpcArgs>
  void send_request(std::string_view method_name, const RpcArgs &...args);

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

template <typename... RpcArgs>
void client_t::send_request(std::string_view method_name,
                            const RpcArgs &...args) {
  msgpack::sbuffer buf;
  msgpack::packer packer{buf};
  auto cmd = std::make_tuple(msg_type_t::request, message_id_++, method_name,
                             std::cref(args)...);
  packer.pack(cmd);
  writer_.write(std::move(buf));
}

} // namespace monitor::neovim
