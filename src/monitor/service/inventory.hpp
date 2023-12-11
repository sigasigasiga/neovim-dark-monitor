#pragma once

#include "monitor/service/monitor.hpp"
#include "monitor/util/inventory.hpp"

namespace monitor::service {

util::inventory_t
make_inventory(boost::asio::any_io_executor exec,
               boost::asio::local::stream_protocol::endpoint singleton_endpoint,
               boost::asio::generic::stream_protocol::socket nvim_socket,
               monitor_t::notifier_t &notifier);

} // namespace monitor::service
