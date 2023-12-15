#pragma once

#include "monitor/neovim/proto/msg_type.hpp"

namespace monitor::neovim::proto {

template <typename T>
concept rpc_method =
    std::same_as<std::decay_t<decltype(T::method_name)>, std::string_view> &&
    requires(T data, msgpack::packer<msgpack::sbuffer> packer) {
      packer.pack(data);
    };

template <typename T>
  requires rpc_method<T> || std::same_as<T, msgpack::object>
struct message_t {
  msg_type_t msg_type;
  std::int64_t message_id;
  std::string method_name;
  T data;

  MSGPACK_DEFINE_ARRAY(msg_type, message_id, method_name, data);
};

} // namespace monitor::neovim::proto
