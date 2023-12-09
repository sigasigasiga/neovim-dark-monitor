#include "monitor/application/application.hpp"

#include <iostream>

#include "monitor/application/monitor.hpp"
#include "monitor/application/singleton_client.hpp"
#include "monitor/util/error.hpp"
#include "monitor/util/os/user.hpp"

namespace monitor::application {

namespace {

constexpr char help_opt[] = "help";
constexpr char nvim_sock_opt[] = "nvim-sock";

namespace po = boost::program_options;

po::options_description make_options_description() {
  po::options_description desc{"neovim-dark-monitor options"};
  desc.add_options()                     //
      (help_opt, "produce help message") //
      (nvim_sock_opt, po::value<std::string>(),
       "neovim socket (defaults to $NVIM)");

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

} // anonymous namespace

application_t::application_t(int argc, const char *argv[])
    : options_description_{make_options_description()},
      cmd_line_{parse_command_line(argc, argv, options_description_)},
      singleton_socket_{make_singleton_socket_path().string()},
      nvim_socket_{cmd_line_.count(nvim_sock_opt)
                       ? cmd_line_.at(nvim_sock_opt).as<std::string>()
                       : get_nvim_socket()} {}

int application_t::run() noexcept try {
  if (cmd_line_.count(help_opt)) {
    std::cout << options_description_ << std::endl;
    return 0;
  }

  spdlog::info("Using {} neovim socket", nvim_socket_);

  if (auto c =
          std::make_unique<singleton_client_t>(singleton_socket_, nvim_socket_);
      c->try_client_mode()) {
    state_ = std::move(c);
  } else {
    state_ = std::make_unique<monitor_t>(singleton_socket_, nvim_socket_);
  }

  assert(state_);

  state_->run();
  return 0;
} catch (monitor::util::error_t error) {
  spdlog::error(monitor::util::error_to_string(error));
  return static_cast<int>(error);
} catch (const std::exception &ex) {
  spdlog::error(ex.what());
  return static_cast<int>(monitor::util::error_t::unhandled_exception);
} catch (...) {
  spdlog::error("Unexpected error");
  return static_cast<int>(monitor::util::error_t::unexpected_error);
}

} // namespace monitor::application
