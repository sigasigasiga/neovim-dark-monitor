#pragma once

#include "monitor/rpc/dispatcher.hpp"
#include "monitor/rpc/write.hpp"
#include "monitor/util/signal_ref.hpp"

namespace monitor::rpc {

// msgpack-rpc client implementation
// https://github.com/msgpack-rpc/msgpack-rpc/blob/master/spec.md
class client_t : public dispatcher_t::delegate_t, public write_t<>::delegate_t {
public:
  class delegate_t {
  public:
    virtual ~delegate_t() = default;
    virtual void on_client_error(std::exception_ptr ep) = 0;
  };

  using result_t = tl::expected<msgpack::object_handle /* result */,
                                msgpack::object_handle /* error */>;

  using response_cb_t = std::function<void(result_t)>;

  class request_handler_t {
  public:
    virtual ~request_handler_t() = default;
    virtual void on_request(std::string_view method,
                            msgpack::object_handle data,
                            response_cb_t send_response) = 0;
  };

  using notification_signal_t =
      util::signal_ref_t<std::string_view /* method */,
                         msgpack::object /* params */>;

public:
  client_t(delegate_t &delegate, request_handler_t &request_handler,
           boost::asio::generic::stream_protocol::socket socket);

public:
  void send_request(std::string_view method, msgpack::object params,
                    response_cb_t cb);

  notification_signal_t notification_signal();

private: // dispatcher_t::delegate_t
  void on_request(request_t request, zone_ptr_t zone) final;
  void on_response(response_t response, zone_ptr_t zone) final;
  void on_notification(notification_t notification, zone_ptr_t zone) final;
  void on_dispatcher_error(std::exception_ptr ep) final;

private: // write_t<>::delegate_t
  void on_write_error(const boost::system::error_code &ec) final;

private:
  delegate_t &delegate_;
  request_handler_t &request_handler_;

  boost::asio::generic::stream_protocol::socket socket_;
  write_t<> writer_;
  dispatcher_t dispatcher_;

  msg_id_t msgid_ = 0;
  std::map<msg_id_t, response_cb_t> request_cbs_;
  notification_signal_t::sig_t notification_signal_;
};

} // namespace monitor::rpc
