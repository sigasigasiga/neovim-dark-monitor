#pragma once

#include "monitor/util/access_tag.hpp"

namespace monitor::util {

template <typename T>
class shared_from_this_base_t : public std::enable_shared_from_this<T> {
protected:
  using sft_base_t = shared_from_this_base_t<T>;
  using sft_tag_t = access_tag_t<sft_base_t>;

protected:
  shared_from_this_base_t(sft_tag_t) {}

public:
  template <typename... Args> static auto make(Args &&...args) {
    static_assert(std::convertible_to<T *, std::enable_shared_from_this<T> *>);
    return std::make_shared<T>(sft_tag_t{}, std::forward<Args>(args)...);
  }
};

} // namespace monitor::util
