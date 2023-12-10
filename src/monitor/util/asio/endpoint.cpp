#include "monitor/util/asio/endpoint.hpp"

#include <charconv>

namespace monitor::util::asio {

std::optional<boost::asio::generic::stream_protocol::endpoint>
make_endpoint(std::string_view address) {
  if (std::filesystem::path{address}.is_absolute()) {
    return boost::asio::local::stream_protocol::endpoint{address};
  }

  const auto colon_idx = address.rfind(':');
  if (colon_idx == std::string_view::npos) {
    return std::nullopt;
  }

  boost::system::error_code ip_ec;
  const auto ip =
      boost::asio::ip::make_address(address.substr(0, colon_idx), ip_ec);
  if (ip_ec) {
    return std::nullopt;
  }

  const auto port_str = address.substr(colon_idx + 1);
  uint16_t port;
  auto [_, port_ec] =
      std::from_chars(port_str.data(), port_str.data() + port_str.size(), port);
  if (port_ec != std::errc{}) {
    return std::nullopt;
  }

  return boost::asio::ip::tcp::endpoint{ip, port};
}

} // namespace monitor::util::asio
