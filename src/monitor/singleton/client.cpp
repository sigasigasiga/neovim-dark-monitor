#include "monitor/singleton/client.hpp"

namespace monitor::singleton {

client_t::client_t(state_dumper_t &state_dumper,
                   boost::asio::generic::stream_protocol::socket socket)
    : state_dumper_{state_dumper}, socket_{std::move(socket)} {
  auto state = state_dumper_.dump_state();
  boost::asio::write(socket_, boost::asio::buffer(state.data(), state.size()));
}

} // namespace monitor::singleton
