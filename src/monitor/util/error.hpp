#pragma once

namespace monitor::util {

// An `error_code_enum` which is used as an application exit code.
// Ideally, it should be an `error_condition_code` instead of `error_code_enum`,
// but unfortunately `std::system_error` is not constructible from those.
// TODO: implement my own exception type?
enum class error_t : int {
  not_nvim_job = 1,
  bad_cmdline_option = 2,
  bad_rpc = 3,
  thirdparty_exception = 124,
  unexpected_error = 125
};

constexpr std::string_view error_to_string(error_t error) {
  switch (error) {
  case error_t::not_nvim_job: {
    return "The process was not started as a neovim job. "
           "See `--help` or `Job_control` documentation page in neovim";
  }

  case error_t::thirdparty_exception: {
    return "Thirdparty exception";
  }

  case error_t::bad_cmdline_option: {
    return "Bad command line option";
  }

  case error_t::bad_rpc: {
    return "Invalid RPC protocol";
  }

  case error_t::unexpected_error: {
    return "Unexpected error";
  }
  }

  __builtin_unreachable(); // FIXME: make it compiler-independent
}

class exception_t : public std::runtime_error {
public:
  exception_t(error_t error)
      : std::runtime_error{static_cast<std::string>(error_to_string(error))},
        error_{error} {}

  exception_t(error_t error, std::string_view suffix)
      : std::runtime_error{fmt::format("{}: {}", suffix,
                                       error_to_string(error))},
        error_{error} {}

public:
  error_t code() const noexcept { return error_; }

private:
  error_t error_;
};

inline const char *what(std::exception_ptr ep) {
  assert(ep);
  try {
    std::rethrow_exception(ep);
  } catch (const std::exception &ex) {
    return ex.what();
  } catch (...) {
    return "Unknown exception";
  }
}

inline thread_local std::error_code ignore_std_error;

} // namespace monitor::util
