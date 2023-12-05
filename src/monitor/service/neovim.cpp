#include "monitor/service/neovim.hpp"

namespace monitor::service {

namespace {

using appearance_t = neovim_t::dark_notifier_t::appearance_t;

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

neovim_t::neovim_t(boost::asio::any_io_executor exec,
                   boost::asio::generic::stream_protocol::socket current_client,
                   dark_notifier_t &notifier)
    : exec_{std::move(exec)}, notifier_{notifier},
      dark_notifier_connection_{notifier_.subscribe(
          std::bind_front(&neovim_t::on_theme_change, this))} {
  clients_.make_job(std::move(current_client));
}

// singleton::server_t::msg_handler_t
void neovim_t::on_client_msg(msgpack::object_handle handle) {
  auto socket_path = handle.get().as<std::string>();
  boost::asio::local::stream_protocol::endpoint endpoint{
      std::move(socket_path)};
  auto socket_ptr =
      std::make_unique<boost::asio::local::stream_protocol::socket>(exec_);
  auto &socket_ref = *socket_ptr;

  auto cb = [this, socket_ptr = std::move(socket_ptr)](
                const boost::system::error_code &ec) {
    if (ec) {
      spdlog::error("Cannot connect to the socket: {} ({})", ec.message(),
                    ec.value());
    } else {
      auto &client = clients_.make_job(std::move(*socket_ptr));
      update_theme(client, notifier_.query());
    }
  };

  socket_ref.async_connect(endpoint, wrap(std::move(cb)));
}

// dark_notifier_t signal slots
void neovim_t::on_theme_change(appearance_t appearance) {
  spdlog::info("Theme was changed to {}", (int)appearance);

  for (auto &client : clients_.range()) {
    update_theme(client, appearance);
  }
}

} // namespace monitor::service
