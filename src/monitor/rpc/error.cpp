#include "monitor/rpc/error.hpp"

#include "monitor/util/error.hpp"

namespace monitor::rpc {

util::exception_t make_rpc_exception(std::string_view str) {
  return util::exception_t{util::error_t::bad_rpc, str};
}

std::exception_ptr make_rpc_ep(std::string_view msg) {
  return std::make_exception_ptr(make_rpc_exception(msg));
}

} // namespace monitor::rpc
