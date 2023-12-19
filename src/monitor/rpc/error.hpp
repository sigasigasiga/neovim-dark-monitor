#pragma once

#include "monitor/util/error.hpp"

namespace monitor::rpc {

util::exception_t make_rpc_exception(std::string_view str);
std::exception_ptr make_rpc_ep(std::string_view msg);

} // namespace monitor::rpc
