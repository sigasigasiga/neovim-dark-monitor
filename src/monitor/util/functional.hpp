#pragma once

namespace monitor::util {

class first_flag_t {
public:
  constexpr bool get() noexcept { return std::exchange(value_, false); }
  constexpr bool operator()() noexcept { return get(); }

private:
  bool value_ = true;
};

} // namespace monitor::util
