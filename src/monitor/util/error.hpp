#pragma once

namespace monitor::util {

enum class error_t : int {
  not_nvim_job = 1,
  unhandled_exception = 2,
  unexpected_error = 125
};

constexpr std::string_view error_to_string(error_t error) {
  switch (error) {
  case error_t::not_nvim_job: {
    return "The process was not started as a neovim job. See `Job_control` "
           "documentation page in neovim";
  }

  case error_t::unhandled_exception: {
    return "Unhandled exception error";
  }

  case error_t::unexpected_error: {
    return "Unexpected error";
  }
  }

  __builtin_unreachable(); // FIXME: make it compiler-independent
}

inline bool is_disconnected(const boost::system::error_code &ec) {
  return ec == boost::asio::error::eof ||
         ec == boost::asio::error::connection_reset;
}

inline thread_local std::error_code std_ignore_error;

} // namespace monitor::util
