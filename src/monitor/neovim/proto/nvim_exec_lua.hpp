#pragma once

namespace monitor::neovim::proto {

template <typename Args = std::array<msgpack::type::nil_t, 0>>
struct nvim_exec_lua_t {
public:
  inline constexpr static std::string_view method_name = "nvim_exec_lua";

public:
  using response_result_t = msgpack::object;

public:
  std::string_view code;
  Args args;

  MSGPACK_DEFINE_ARRAY(code, args);
};

} // namespace monitor::neovim::proto
