#pragma once

#include "monitor/rpc/message.hpp"
#include "monitor/rpc/read.hpp"

namespace monitor::rpc {

class dispatcher_t : public read_t<>::delegate_t {
public:
  class delegate_t {
  public:
    using zone_ptr_t = msgpack::unique_ptr<msgpack::zone>;

  public:
    virtual ~delegate_t() = default;

  public:
    virtual void on_request(request_t request, zone_ptr_t zone) = 0;
    virtual void on_response(response_t response, zone_ptr_t zone) = 0;
    virtual void on_notification(notification_t notification,
                                 zone_ptr_t zone) = 0;

    virtual void on_dispatcher_error(std::exception_ptr ep) = 0;
  };

public:
  dispatcher_t(delegate_t &delegate,
               boost::asio::generic::stream_protocol::socket &socket);

private: // read_t<>::delegate_t
  void on_message_received(msgpack::object_handle handle) final;
  void on_read_error(const boost::system::error_code &ec) final;

private:
  delegate_t &delegate_;
  read_t<> reader_;
};

} // namespace monitor::rpc
