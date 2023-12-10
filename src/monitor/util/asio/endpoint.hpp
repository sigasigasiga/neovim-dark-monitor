#pragma once

namespace monitor::util::asio {

std::optional<boost::asio::generic::stream_protocol::endpoint>
make_endpoint(std::string_view address);

} // namespace monitor::util::asio
