#pragma once

namespace monitor::neovim::proto {

struct nvim_get_api_info_t {
public:
  inline constexpr static std::string_view method_name = "nvim_get_api_info";

public:
  struct response_result_t {
    std::uintmax_t channel_id;
    msgpack::object api_metadata;

    MSGPACK_DEFINE_ARRAY(channel_id, api_metadata);
  };

public:
  MSGPACK_DEFINE_ARRAY();
};

} // namespace monitor::neovim::proto
