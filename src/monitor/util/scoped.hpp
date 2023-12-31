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

class move_only_t {
public:
  constexpr move_only_t() = default;

  constexpr move_only_t(const move_only_t &) = delete;
  constexpr move_only_t &operator=(const move_only_t &) = delete;

  constexpr move_only_t(move_only_t &&) = default;
  constexpr move_only_t &operator=(move_only_t &&) = default;
};

} // namespace monitor::util
