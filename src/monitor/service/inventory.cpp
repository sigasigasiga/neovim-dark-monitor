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
  auto request_handler = std::make_unique<request_handler_t>(query);
  auto neovim = std::make_unique<neovim_t>(neovim_delegate, *request_handler,
                                           *request_handler, exec,
                                           std::move(nvim_socket));
  auto singleton = std::make_unique<singleton_t>(
      exec, std::move(singleton_endpoint), *neovim);
  auto monitor =
      std::make_unique<monitor_t>(appearance_signal, neovim->clients());

  std::list<std::unique_ptr<util::service_t>> services;
  services.push_back(std::move(request_handler));
  services.push_back(std::move(neovim));
  services.push_back(std::move(singleton));
  services.push_back(std::move(monitor));

  return util::inventory_t{exec, std::move(services)};
}

} // namespace monitor::service
