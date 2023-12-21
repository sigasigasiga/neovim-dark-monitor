#pragma once

namespace monitor::neovim::proto {

struct nvim_set_client_info_t {
public:
  inline constexpr static std::string_view method_name = "nvim_set_client_info";

public:
  using response_result_t = msgpack::type::nil_t;

  struct version_t {
    std::optional<std::uintmax_t> major, minor, patch;
    std::optional<std::string_view> prerelease, commit;

    MSGPACK_DEFINE_MAP(major, minor, patch, prerelease, commit);
  };

  struct method_properties_t {
    std::optional<bool> async;
    std::optional<std::uintmax_t> nargs;

    MSGPACK_DEFINE_MAP(async, nargs);
  };

  using methods_t = std::map<std::string_view, method_properties_t>;

  using attributes_t = std::map<std::string_view, std::string_view>;

public:
  std::string_view name;
  version_t version;
  std::string_view type;
  methods_t methods;
  attributes_t attributes;

  MSGPACK_DEFINE_ARRAY(name, version, type, methods, attributes);
};

} // namespace monitor::neovim::proto
