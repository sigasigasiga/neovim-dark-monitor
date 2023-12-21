#include "monitor/service/request_handler.hpp"

#include "monitor/neovim/proto/nvim_exec_autocmds.hpp"
#include "monitor/neovim/proto/nvim_exec_lua.hpp"
#include "monitor/neovim/proto/nvim_get_api_info.hpp"
#include "monitor/neovim/proto/nvim_set_client_info.hpp"
#include "monitor/neovim/send.hpp"
#include "monitor/util/shared_from_this_base.hpp"

namespace monitor::service {

namespace {

struct autocmds_options_t {
  std::string_view pattern;
  MSGPACK_DEFINE_MAP(pattern);
};

template <neovim::method Method>
void ignore_result(neovim::method_response_t<Method> response) {
  std::ignore = response;
}

template <neovim::method Method>
void send_one_way_message(rpc::client_t &client, const Method &method) {
  neovim::send(client, method, &ignore_result<Method>);
}

using namespace neovim::proto;

class client_initializer_t
    : public util::shared_from_this_base_t<client_initializer_t> {
public:
  client_initializer_t(sft_tag_t tag, rpc::client_t &client)
      : sft_base_t{tag}, client_{client} {}

public:
  void start() {
    neovim::send(client_, nvim_get_api_info_t{},
                 std::bind_front(&client_initializer_t::on_get_api_info,
                                 shared_from_this()));
  }

private:
  template <typename T> using resp_t = neovim::method_response_t<T>;

private:
  void on_get_api_info(resp_t<nvim_get_api_info_t> response) {
    if (!response) {
      return;
    }

    const auto script =
        fmt::format("DarkMonitorQuery = function() "
                    "  return vim.rpcrequest({}, 'DarkMonitorQuery') "
                    "end",
                    response->channel_id);

    neovim::send(client_, nvim_exec_lua_t<>{.code = script},
                 std::bind_front(&client_initializer_t::on_exec_lua,
                                 shared_from_this()));
  }

  void on_exec_lua(resp_t<nvim_exec_lua_t<>> response) {
    if (!response) {
      return;
    }

    const autocmds_options_t options{.pattern = "OnDarkMonitorConnected"};
    const neovim::proto::nvim_exec_autocmds_t exec_autocmd{"User", options};
    send_one_way_message(client_, exec_autocmd);
  }

private:
  rpc::client_t &client_;
};

} // anonymous namespace

request_handler_t::request_handler_t(query_t &query) : query_{query} {}

// neovim_t::client_init_t
void request_handler_t::on_new_client(rpc::client_t &client) {
  send_one_way_message(
      client, nvim_set_client_info_t{.name = "DarkMonitor", .type = "remote"});

  const auto initializer = client_initializer_t::make(client);
  initializer->start();
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
