#include "monitor/rpc/client.hpp"

#include "monitor/rpc/error.hpp"

namespace monitor::rpc {

client_t::client_t(delegate_t &delegate, request_handler_t &request_handler,
                   boost::asio::generic::stream_protocol::socket socket)
    : delegate_{delegate}, request_handler_{request_handler}, socket_{std::move(
                                                                  socket)},
      writer_{*this, socket_}, dispatcher_{*this, socket_} {}

void client_t::send_request(std::string_view method, msgpack::object params,
                            response_cb_t cb) {
  assert(params.type == msgpack::type::ARRAY);

  const auto id = msgid_++;
  // Well, technically it is possible that an insertion didn't take the place,
  // if `msgid_` was overflown and the previous requests hadn't been responsed
  // to yet, but it is considered almost impossible in practice
  assert(!request_cbs_.contains(id));
  request_cbs_.insert(std::pair{id, std::move(cb)});

  request_t request{.msgid = id, .method = method, .params = params};

  msgpack::zone zone;
  msgpack::object request_object{request, zone};
  writer_.write(request_object);
}

auto client_t::notification_signal() -> notification_signal_t {
  return notification_signal_;
}

// dispatcher_t::delegate_t
void client_t::on_request(request_t request, zone_ptr_t zone) {
  if (request.params.type != msgpack::type::ARRAY) {
    return delegate_.on_client_error(
        make_rpc_ep("Request params is not an array"));
  }

  msgpack::object_handle params{request.params, std::move(zone)};
  request_handler_.on_request(request.method, std::move(params),
                              [this, msgid = request.msgid](result_t result) {
                                response_t response{
                                    .msgid = msgid,
                                };
                                if (result) {
                                  response.result = result->get();
                                } else {
                                  response.error = result.error().get();
                                }

                                msgpack::zone zone;
                                msgpack::object response_object{response, zone};
                                writer_.write(response_object);
                              });
}

void client_t::on_response(response_t response, zone_ptr_t zone) {
  const auto cb_handle = request_cbs_.extract(response.msgid);

  if (cb_handle.empty()) {
    return delegate_.on_client_error(make_rpc_ep(fmt::format(
        "Got a response for id {}, but it was not found", response.msgid)));
  }

  const auto &result = response.result, &error = response.error;
  if (result.type != msgpack::type::NIL && error.type != msgpack::type::NIL) {

    return delegate_.on_client_error(
        make_rpc_ep("Error and result are both not null"));
  }

  if (response.error.type == msgpack::type::NIL) {
    msgpack::object_handle result_handle{response.result, std::move(zone)};
    boost::asio::post(socket_.get_executor(),
                      std::bind_front(std::move(cb_handle.mapped()),
                                      std::move(result_handle)));
  } else {
    msgpack::object_handle error_handle{response.error, std::move(zone)};
    result_t error{tl::unexpect, std::move(error_handle)};
    boost::asio::post(
        socket_.get_executor(),
        std::bind_front(std::move(cb_handle.mapped()), std::move(error)));
  }
}

void client_t::on_notification(notification_t notification, zone_ptr_t zone) {
  std::ignore = zone;
  notification_signal_(notification.method, notification.params);
}

void client_t::on_dispatcher_error(std::exception_ptr ep) {
  assert(ep);
  delegate_.on_client_error(ep);
}

// write_t<>::delegate_t
void client_t::on_write_error(const boost::system::error_code &ec) {
  assert(ec);
  delegate_.on_client_error(std::make_exception_ptr(
      boost::system::system_error{ec, "RPC client write"}));
}

} // namespace monitor::rpc
