#include "monitor/service/inventory.hpp"

#include "monitor/service/neovim.hpp"
#include "monitor/service/singleton.hpp"

namespace monitor::service {

util::inventory_t
make_inventory(boost::asio::any_io_executor exec,
               boost::asio::local::stream_protocol::endpoint singleton_endpoint,
               boost::asio::generic::stream_protocol::socket nvim_socket,
               neovim_t::dark_notifier_t &notifier) {
  auto neovim =
      std::make_unique<neovim_t>(exec, std::move(nvim_socket), notifier);
  auto singleton = std::make_unique<singleton_t>(
      exec, std::move(singleton_endpoint), *neovim);

  std::list<std::unique_ptr<util::service_t>> services;
  services.push_back(std::move(neovim));
  services.push_back(std::move(singleton));

  return util::inventory_t{exec, std::move(services)};
}

} // namespace monitor::service
