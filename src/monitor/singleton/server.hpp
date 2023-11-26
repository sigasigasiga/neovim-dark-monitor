#pragma once

#include "monitor/singleton/client_handler.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/job.hpp"

namespace monitor::singleton {

class server_t : private util::callback_wrapper_t {
public:
  using msg_handler_t = client_handler_t::msg_handler_t;

public:
  server_t(msg_handler_t &msg_handler, boost::asio::any_io_executor exec,
           boost::asio::local::stream_protocol::endpoint ep);

public:
  void start();

private:
  void async_accept();
  void handle_accept(const boost::system::error_code &ec,
                     boost::asio::local::stream_protocol::socket client_socket);

private:
  msg_handler_t &msg_handler_;
  boost::asio::any_io_executor exec_;
  boost::asio::local::stream_protocol::acceptor acceptor_;

  util::job_storage_t<client_handler_t> jobs_;
};

} // namespace monitor::singleton
