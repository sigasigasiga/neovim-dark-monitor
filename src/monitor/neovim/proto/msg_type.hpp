#pragma once

namespace monitor::neovim::proto {

enum class msg_type_t : std::int64_t {
  request = 0,
  response = 1,
  notify = 2,
};

} // namespace monitor::neovim::proto

// this macro should be in the global namespace
MSGPACK_ADD_ENUM(monitor::neovim::proto::msg_type_t);
