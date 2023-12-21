#pragma once

namespace monitor::util {

template <typename T> class access_tag_t {
private:
  explicit constexpr access_tag_t() = default;

  friend T;
};

} // namespace monitor::util
