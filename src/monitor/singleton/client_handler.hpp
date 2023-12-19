#pragma once

#include "monitor/rpc/read.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/job.hpp"

namespace monitor::singleton {

class client_handler_t : public util::job_t,
                         private rpc::read_t<>::delegate_t,
                         private util::callback_wrapper_t {
public:
  class msg_handler_t {
  public:
    virtual ~msg_handler_t() = default;
    virtual void on_client_msg(msgpack::object_handle handle) = 0;
  };

public:
  client_handler_t(std::unique_ptr<job_t::delegate_t> job_delegate,
                   msg_handler_t &msg_handler,
                   boost::asio::generic::stream_protocol::socket client_socket);

private: // rpc::read_t<>::delegate_t
  void on_message_received(msgpack::object_handle handle) final;
  void on_read_error(const boost::system::error_code &ec) final;

private:
  msg_handler_t &msg_handler_;
  boost::asio::generic::stream_protocol::socket client_socket_;
  rpc::read_t<> reader_;
};

} // namespace monitor::singleton
