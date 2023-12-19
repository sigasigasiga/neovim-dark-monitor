#include "monitor/service/monitor.hpp"

#include "monitor/neovim/proto/nvim_exec_autocmds.hpp"
#include "monitor/neovim/send.hpp"

namespace monitor::service {

namespace {

struct autocmds_options_t {
  std::string_view pattern;
  std::string_view data;

  MSGPACK_DEFINE_MAP(pattern, data);
};

} // anonymous namespace

monitor_t::monitor_t(appearance_signal_t sig,
                     ranges::any_view<rpc::client_t &> neovim_clients)
    : neovim_clients_{std::move(neovim_clients)},
      appearance_connection_{
          sig.subscribe(std::bind_front(&monitor_t::on_appearance, this))} {}

// notifier_t signal handlers
void monitor_t::on_appearance(appearance_t appearance) {
  spdlog::info("Theme was changed to {}", (int)appearance);

  for (auto &client : neovim_clients_) {
    const autocmds_options_t options{.pattern = "OnDarkMonitorThemeChange",
                                     .data = to_string(appearance)};
    const neovim::proto::nvim_exec_autocmds_t cmd{"User", options};

    neovim::send(
        client, cmd, [](neovim::method_response_t<decltype(cmd)> response) {
          if (response) {
            spdlog::trace(
                "Successfully executed autocommand `OnDarkMonitorThemeChange`");
          } else {
            std::ostringstream oss;
            oss << response.error().get();
            spdlog::error(
                "Unable to execute autocommand `OnDarkMonitorThemeChange`: {}",
                oss.str());
          }
        });
  }
}

} // namespace monitor::service
