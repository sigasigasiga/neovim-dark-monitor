#pragma once

#include "monitor/service/monitor.hpp"
#include "monitor/service/neovim.hpp"
#include "monitor/service/request_handler.hpp"
#include "monitor/util/inventory.hpp"

namespace monitor::service {

util::inventory_t
make_inventory(boost::asio::any_io_executor exec,
               boost::asio::local::stream_protocol::endpoint singleton_endpoint,
               boost::asio::generic::stream_protocol::socket nvim_socket,
               request_handler_t::query_t &query,
               neovim_t::delegate_t &neovim_delegate,
               monitor_t::appearance_signal_t appearance_signal);

} // namespace monitor::service
