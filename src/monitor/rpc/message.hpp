#pragma once

#include "monitor/util/meta.hpp"

namespace monitor::rpc {

enum class msg_type_t : std::int64_t {
  request = 0,
  response = 1,
  notification = 2,
};

} // namespace monitor::rpc

// this macro should be in the global namespace
MSGPACK_ADD_ENUM(monitor::rpc::msg_type_t);

namespace monitor::rpc {

using msg_id_t = std::uint32_t;

struct request_t {
  msg_type_t type = msg_type_t::request;
  msg_id_t msgid;
  std::string_view method;
  msgpack::object params;

  MSGPACK_DEFINE_ARRAY(type, msgid, method, params);
};

struct response_t {
  msg_type_t type = msg_type_t::response;
  msg_id_t msgid;
  msgpack::object error;
  msgpack::object result;

  MSGPACK_DEFINE_ARRAY(type, msgid, error, result);
};

struct notification_t {
  msg_type_t type = msg_type_t::notification;
  std::string_view method;
  msgpack::object params;

  MSGPACK_DEFINE_ARRAY(type, method, params);
};

template <typename T>
concept message = util::one_of<T, request_t, response_t, notification_t>;

std::variant<request_t, response_t, notification_t, std::exception_ptr>
make_message(msgpack::object object) noexcept;

} // namespace monitor::rpc
