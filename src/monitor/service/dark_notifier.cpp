#include "monitor/service/dark_notifier.hpp"

namespace monitor::service {

namespace {

using appearance_t = siga::dark_notify::dark_notify_t::appearance_t;

std::vector<std::string> make_commands(appearance_t appearance) {
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

} // anonymous namespace

dark_notifier_t::dark_notifier_t(
    boost::asio::any_io_executor exec,
    const util::job_storage_t<neovim::client_t> &neovim_jobs)
    : neovim_jobs_{neovim_jobs}, dark_notifier_{std::move(exec)},
      connection_{dark_notifier_.subscribe(
          std::bind_front(&dark_notifier_t::on_theme_change, this))} {}

// util::service_t
void dark_notifier_t::reload() {
  if (first_.get()) {
    dark_notifier_.start();
  }
}

// dark_notifier::dark_notifier_t slots
void dark_notifier_t::on_theme_change(appearance_t appearance) {
  spdlog::info("Theme was changed to {}", (int)appearance);

  auto commands = make_commands(appearance);
  for (auto &neovim : neovim_jobs_.range()) {
    neovim.send_request("nvim_command", commands);
  }
}

} // namespace monitor::service
