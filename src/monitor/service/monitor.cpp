#include "monitor/service/monitor.hpp"

namespace monitor::service {

namespace {

using appearance_t = monitor_t::notifier_t::appearance_t;

std::array<std::string_view, 1> make_commands(appearance_t appearance) {
  // TODO: this should be configurable
  switch (appearance) {
  case appearance_t::light: {
    return {"set background=light | colorscheme dawnfox"};
  }

  case appearance_t::dark: {
    return {"set background=dark | colorscheme gruvbox"};
  }

  case appearance_t::unknown: {
    return {"echo 'neovim-dark-monitor: system appearance is unknown'"};
  }
  }
}

void update_theme(neovim::client_t &client, appearance_t appearance) {
  client.send_request("nvim_command", make_commands(appearance));
}

} // anonymous namespace

monitor_t::monitor_t(
    notifier_t &notifier,
    const util::job_storage_t<neovim::client_t> &neovim_clients,
    util::signal_ref_t<neovim::client_t &> nvim_client_sig)
    : notifier_{notifier}, neovim_clients_{neovim_clients},
      appearance_connection_{notifier_.signal().subscribe(
          std::bind_front(&monitor_t::on_appearance, this))},
      neovim_connection_{nvim_client_sig.subscribe(
          std::bind_front(&monitor_t::on_neovim_client, this))} {}

// util::service_t
void monitor_t::reload() {
  if (!first_()) {
    return;
  }

  for (auto &client : neovim_clients_.range()) {
    update_theme(client, notifier_.query());
  }
}

// notifier_t signal handlers
void monitor_t::on_appearance(notifier_t::appearance_t appearance) {
  spdlog::info("Theme was changed to {}", (int)appearance);

  for (auto &client : neovim_clients_.range()) {
    update_theme(client, appearance);
  }
}

// neovim_client_sig_t signal handlers
void monitor_t::on_neovim_client(neovim::client_t &client) {
  update_theme(client, notifier_.query());
}

} // namespace monitor::service
