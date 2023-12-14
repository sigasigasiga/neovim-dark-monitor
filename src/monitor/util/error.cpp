#include "monitor/util/error.hpp"

namespace monitor::util {

const std::error_category &monitor_error_category() {
  class monitor_error_category_t : public std::error_category {
  private: // std::error_category
    const char *name() const noexcept final { return "monitor error category"; }

    std::string message(int condition) const final {
      const auto err = static_cast<error_t>(condition);
      return static_cast<std::string>(error_to_string(err));
    }
  };

  static monitor_error_category_t ret;
  return ret;
}

std::error_code make_error_code(error_t error) {
  return std::error_code{static_cast<int>(error), monitor_error_category()};
}

} // namespace monitor::util
