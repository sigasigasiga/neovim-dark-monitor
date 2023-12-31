#include "monitor/service/inventory.hpp"

#include "monitor/service/neovim.hpp"
#include "monitor/service/singleton.hpp"

namespace monitor::service {

util::inventory_t
make_inventory(boost::asio::any_io_executor exec,
               boost::asio::local::stream_protocol::endpoint singleton_endpoint,
               boost::asio::generic::stream_protocol::socket nvim_socket,
               request_handler_t::query_t &query,
               neovim_t::delegate_t &neovim_delegate,
               monitor_t::appearance_signal_t appearance_signal) {
  util::inventory_builder_t builder{exec};
  auto &request_handler = builder.add_service<request_handler_t>(query);
  auto &neovim = builder.add_service<neovim_t>(neovim_delegate, request_handler,
                                               request_handler, exec,
                                               std::move(nvim_socket));
  std::ignore = builder.add_service<singleton_t>(
      exec, std::move(singleton_endpoint), neovim);
  std::ignore =
      builder.add_service<monitor_t>(appearance_signal, neovim.clients());

  return builder.make_inventory();
}

} // namespace monitor::service
