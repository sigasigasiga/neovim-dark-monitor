#include "monitor/service/request_handler.hpp"

#include "monitor/neovim/proto/nvim_exec_autocmds.hpp"
#include "monitor/neovim/send.hpp"

namespace monitor::service {

namespace {

struct autocmds_options_t {
  std::string_view pattern;
  MSGPACK_DEFINE_MAP(pattern);
};

} // anonymous namespace

request_handler_t::request_handler_t(query_t &query) : query_{query} {}

// neovim_t::client_init_t
void request_handler_t::on_new_client(rpc::client_t &client) {
  // TODO:
  // 0. Set client info using `nvim_set_client_info`
  // 1. Register vim-command `query` that would query the info (use
  //    `nvim_get_api_info`)

  const autocmds_options_t options{.pattern = "OnDarkMonitorConnected"};
  const neovim::proto::nvim_exec_autocmds_t cmd{"User", options};
  neovim::send(
      client, cmd, [](neovim::method_response_t<decltype(cmd)> response) {
        if (response) {
          spdlog::trace(
              "Successfully executed autocommand `OnDarkMonitorConnected`");
        } else {
          std::ostringstream oss;
          oss << response.error().get();
          spdlog::error(
              "Unable to execute autocommand `OnDarkMonitorConnected`: {}",
              oss.str());
        }
      });
}

// rpc::client_t::request_handler_t
void request_handler_t::on_request(std::string_view method,
                                   msgpack::object_handle data,
                                   rpc::client_t::response_cb_t send_response) {
  auto zone_ptr = std::make_unique<msgpack::zone>();
  if (method == "DarkMonitorQuery") {
    msgpack::object obj{to_string(query_.query()), *zone_ptr};
    send_response(msgpack::object_handle{std::move(obj), std::move(zone_ptr)});
  } else {
    const auto msg = fmt::format("`{}` is an unsupported command", method);
    msgpack::object obj{msg, *zone_ptr};
    send_response(tl::unexpected{
        msgpack::object_handle{std::move(obj), std::move(zone_ptr)}});
  }
}

} // namespace monitor::service
