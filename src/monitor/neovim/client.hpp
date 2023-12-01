#pragma once

#include "monitor/neovim/proto.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/job.hpp"

namespace monitor::neovim {

class client_t : public util::job_t, private util::callback_wrapper_t {
public:
  client_t(std::unique_ptr<job_t::delegate_t> job_delegate,
           boost::asio::generic::stream_protocol::socket socket);

public:
  template <typename... RpcArgs>
  void send_request(std::string_view method_name, const RpcArgs &...args);

private:
  void process_queue();
  void handle_write(const boost::system::error_code &ec, std::size_t bytes);

private:
  boost::asio::generic::stream_protocol::socket socket_;
  std::int64_t message_id_ = 0;
  std::deque<msgpack::sbuffer> buffer_queue_;
};

template <typename... RpcArgs>
void client_t::send_request(std::string_view method_name,
                            const RpcArgs &...args) {
  msgpack::sbuffer buf;
  msgpack::packer packer{buf};
  auto cmd = std::make_tuple(msg_type_t::request, message_id_++, method_name,
                             std::cref(args)...);
  packer.pack(cmd);

  buffer_queue_.push_back(std::move(buf));
  if (buffer_queue_.size() == 1) {
    boost::asio::post(socket_.get_executor(),
                      wrap(std::bind_front(&client_t::process_queue, this)));
  }
}

} // namespace monitor::neovim
