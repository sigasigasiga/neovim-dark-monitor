#include "monitor/rpc/dispatcher.hpp"

namespace monitor::rpc {

dispatcher_t::dispatcher_t(
    delegate_t &delegate, boost::asio::generic::stream_protocol::socket &socket)
    : delegate_{delegate}, reader_{*this, socket} {}

// read_t<>::delegate_t
void dispatcher_t::on_message_received(msgpack::object_handle handle) {
  auto zone = std::move(handle.zone());
  const auto msg = make_message(handle.get());

  class visitor_t {
  public:
    using zone_ptr_t = delegate_t::zone_ptr_t;

  public:
    visitor_t(delegate_t &delegate, zone_ptr_t zone)
        : delegate_{delegate}, zone_{std::move(zone)} {}

  public:
    void operator()(const request_t &request) {
      return delegate_.on_request(request, std::move(zone_));
    }

    void operator()(const response_t &response) {
      return delegate_.on_response(response, std::move(zone_));
    }

    void operator()(const notification_t &notification) {
      return delegate_.on_notification(notification, std::move(zone_));
    }

    void operator()(std::exception_ptr ep) {
      return delegate_.on_dispatcher_error(ep);
    }

  private:
    delegate_t &delegate_;
    zone_ptr_t zone_;
  };

  visitor_t vis{delegate_, std::move(zone)};
  std::visit(vis, msg);
}

void dispatcher_t::on_read_error(const boost::system::error_code &ec) {
  delegate_.on_dispatcher_error(
      std::make_exception_ptr(boost::system::system_error{ec}));
}

} // namespace monitor::rpc
