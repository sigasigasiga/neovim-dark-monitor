#pragma once

namespace monitor::neovim::proto {

template <typename Options> struct nvim_exec_autocmds_t {
public:
  inline constexpr static std::string_view method_name = "nvim_exec_autocmds";

public:
  using response_result_t = msgpack::type::nil_t;

public:
  std::string event;
  Options options;

public:
  MSGPACK_DEFINE_ARRAY(event, options);
};

template <typename Options>
nvim_exec_autocmds_t(std::string, Options &&)
    -> nvim_exec_autocmds_t<std::decay_t<Options>>;

} // namespace monitor::neovim::proto
