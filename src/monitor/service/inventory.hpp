#pragma once

#include "monitor/util/inventory.hpp"

namespace monitor::service {

util::inventory_t
make_inventory(boost::asio::any_io_executor exec,
               boost::asio::local::stream_protocol::endpoint singleton_endpoint,
               std::string nvim_socket);

} // namespace monitor::service
