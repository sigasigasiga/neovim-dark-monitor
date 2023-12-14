#pragma once

namespace monitor::util {

// An `error_code_enum` which is used as an application exit code.
// Ideally, it should be an `error_condition_code` instead of `error_code_enum`,
// but unfortunately `std::system_error` is not constructible from those.
// TODO: implement my own exception type?
enum class error_t : int {
  not_nvim_job = 1,
  unhandled_exception = 2,
  bad_cmdline_option = 3,
  unexpected_error = 125
};

constexpr std::string_view error_to_string(error_t error) {
  switch (error) {
  case error_t::not_nvim_job: {
    return "The process was not started as a neovim job. "
           "See `--help` or `Job_control` documentation page in neovim";
  }

  case error_t::unhandled_exception: {
    return "Unhandled exception error";
  }

  case error_t::bad_cmdline_option: {
    return "Bad command line option";
  }

  case error_t::unexpected_error: {
    return "Unexpected error";
  }
  }

  __builtin_unreachable(); // FIXME: make it compiler-independent
}

const std::error_category &monitor_error_category();
std::error_code make_error_code(error_t error);

inline bool is_disconnected(const boost::system::error_code &ec) {
  return ec == boost::asio::error::eof ||
         ec == boost::asio::error::connection_reset;
}

inline thread_local std::error_code std_ignore_error;

} // namespace monitor::util

template <>
struct std::is_error_code_enum<monitor::util::error_t> : public std::true_type {
};
