#include "monitor/application/application.hpp"

#include <iostream>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "monitor/application/mode/monitor.hpp"
#include "monitor/application/mode/singleton_client.hpp"
#include "monitor/util/error.hpp"
#include "monitor/util/os/user.hpp"

namespace monitor::application {

namespace {

constexpr char help_opt[] = "help,h";
constexpr char nvim_sock_opt[] = "nvim-sock";
constexpr char loglevel_opt[] = "loglevel";

namespace po = boost::program_options;

constexpr std::array loglevel_names = SPDLOG_LEVEL_NAMES;

po::options_description make_options_description() {
  static const auto loglevel_msg =
      fmt::format("available options: {}", fmt::join(loglevel_names, ", "));

  po::options_description desc{"neovim-dark-monitor options"};
  // clang-format off
  desc.add_options()
    (help_opt, "produce help message")
    (nvim_sock_opt, po::value<std::string>(), "neovim socket (defaults to $NVIM)")
    (loglevel_opt, po::value<std::string>(), loglevel_msg.c_str())
  ;
  // clang-format on

  return desc;
}

po::variables_map parse_command_line(int argc, const char *argv[],
                                     const po::options_description &desc) {
  po::variables_map ret;

  po::store(po::parse_command_line(argc, argv, desc), ret);
  po::notify(ret);

  return ret;
}

std::string get_nvim_socket() {
  const char *nvim_socket_path = std::getenv("NVIM");
  if (nvim_socket_path) {
    spdlog::debug("Using NVIM={}", nvim_socket_path);
    return nvim_socket_path;
  } else {
    throw util::error_t::not_nvim_job;
  }
}

std::string make_singleton_socket_filename() {
  return fmt::format("neovim-dark-monitor.{}.sock", util::os::get_user_id());
}

std::filesystem::path make_runtime_path() {
  static const std::filesystem::path default_server_socket_path =
      std::filesystem::temp_directory_path();

  const char *nvim_env = std::getenv("XDG_RUNTIME_DIR");
  if (nvim_env == nullptr) {
    spdlog::warn("$XDG_RUNTIME_DIR is not set, using {} instead",
                 default_server_socket_path.string());
    return default_server_socket_path;
  } else {
    return nvim_env;
  }
}

std::filesystem::path make_singleton_socket_path() {
  return make_runtime_path() / make_singleton_socket_filename();
}

void set_loglevel(const std::string &loglevel_str) {
  if (!ranges::contains(loglevel_names, loglevel_str)) {
    throw util::exception_t{util::error_t::bad_cmdline_option,
                            "Unknown loglevel"};
  }

  const auto loglevel = spdlog::level::from_str(loglevel_str);
  spdlog::set_level(loglevel);
}

} // anonymous namespace

application_t::application_t(int argc, const char *argv[])
    : options_description_{make_options_description()},
      cmd_line_{parse_command_line(argc, argv, options_description_)} {}

int application_t::run() {
  if (cmd_line_.count("help")) {
    std::cout << options_description_ << std::endl;
    return 0;
  }

  if (cmd_line_.count(loglevel_opt)) {
    const auto &loglevel_str = cmd_line_.at(loglevel_opt).as<std::string>();
    set_loglevel(loglevel_str);
  }

  const auto singleton_socket = make_singleton_socket_path().string();
  const auto nvim_socket = cmd_line_.count(nvim_sock_opt)
                               ? cmd_line_.at(nvim_sock_opt).as<std::string>()
                               : get_nvim_socket();

  spdlog::info("Using {} neovim socket", nvim_socket);

  if (auto c = std::make_unique<mode::singleton_client_t>(singleton_socket,
                                                          nvim_socket);
      c->try_client_mode()) {
    state_ = std::move(c);
  } else {
    state_ = std::make_unique<mode::monitor_t>(singleton_socket, nvim_socket);
  }

  assert(state_);

  state_->run();
  return 0;
}

} // namespace monitor::application
