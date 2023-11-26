#include "monitor/service/inventory.hpp"

#include "monitor/service/singleton.hpp"

namespace monitor::service {

util::inventory_t
make_inventory(boost::asio::any_io_executor exec,
               boost::asio::local::stream_protocol::endpoint singleton_endpoint,
               std::string nvim_socket) {
  auto singleton =
      std::make_unique<singleton_t>(exec, std::move(singleton_endpoint));

  std::list<std::unique_ptr<util::service_t>> services;
  services.push_back(std::move(singleton));

  return util::inventory_t{exec, std::move(services)};
}

} // namespace monitor::service
