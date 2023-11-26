#pragma once

#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/job.hpp"
#include "monitor/util/msgpack_socket_read.hpp"

namespace monitor::singleton {

class client_handler_t : public util::job_t,
                         private util::msgpack_socket_read_t::delegate_t,
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

private: // util::msgpack_socket_read_t::delegate_t
  void on_message_received(msgpack::object_handle handle) final;
  void on_error(const boost::system::error_code &ec) final;

private:
  msg_handler_t &msg_handler_;
  util::msgpack_socket_read_t reader_;
};

} // namespace monitor::singleton
