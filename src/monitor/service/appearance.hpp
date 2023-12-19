#pragma once

namespace monitor::service {

enum class appearance_t { unknown, light, dark };

inline std::string_view to_string(appearance_t appearance) {
  switch (appearance) {
  case appearance_t::light: {
    return "light";
  }

  case appearance_t::dark: {
    return "dark";
  }

  case appearance_t::unknown: {
    return "unknown";
  }
  }
}

} // namespace monitor::service
