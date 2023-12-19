#pragma once

namespace monitor::util::asio {

inline bool is_disconnected(const boost::system::error_code &ec) {
  return ec == boost::asio::error::eof ||
         ec == boost::asio::error::connection_reset;
}

inline bool is_disconnected(std::exception_ptr ep) {
  assert(ep);
  try {
    std::rethrow_exception(ep);
  } catch (const boost::system::system_error &ex) {
    return is_disconnected(ex.code());
  } catch (...) {
    return false;
  }
}

} // namespace monitor::util::asio
