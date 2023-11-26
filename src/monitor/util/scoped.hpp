#pragma once

namespace monitor::util {

class scoped_t {
public:
  constexpr scoped_t() = default;

  constexpr scoped_t(const scoped_t &) = delete;
  constexpr scoped_t &operator=(const scoped_t &) = delete;

  constexpr scoped_t(scoped_t &&) = delete;
  constexpr scoped_t &operator=(scoped_t &&) = delete;
};

} // namespace monitor::util
