#include "monitor/rpc/message.hpp"

#include "monitor/rpc/error.hpp"

namespace monitor::rpc {

std::variant<request_t, response_t, notification_t, std::exception_ptr>
make_message(msgpack::object object) noexcept try {
  if (object.type != msgpack::type::ARRAY) {
    throw make_rpc_exception(
        fmt::format("An array was expected, got {}", (int)object.type));
  }

  const auto subobjects = object.as<std::vector<msgpack::object>>();
  if (subobjects.empty()) {
    throw make_rpc_exception("Got an empty array");
  }

  const auto &raw_msg_type = subobjects.front();
  if (raw_msg_type.type != msgpack::type::POSITIVE_INTEGER) {
    throw make_rpc_exception("Bad msg_type type");
  }

  const auto msg_type = raw_msg_type.as<msg_type_t>();
  switch (msg_type) {
  case msg_type_t::request: {
    return object.as<request_t>();
  }

  case msg_type_t::response: {
    return object.as<response_t>();
  }

  case msg_type_t::notification: {
    return object.as<notification_t>();
  }

  default: {
    throw make_rpc_exception("Unexpected msg_type");
  }
  }
} catch (...) {
  return std::current_exception();
}

} // namespace monitor::rpc
