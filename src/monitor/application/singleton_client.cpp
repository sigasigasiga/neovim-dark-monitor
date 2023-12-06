#include "monitor/application/singleton_client.hpp"

namespace monitor::application {

singleton_client_t::singleton_client_t(const std::string &singleton_socket,
                                       const std::string &nvim_socket)
    : state_{std::in_place_index<0>, singleton_socket}, nvim_socket_{
                                                            nvim_socket} {}

bool singleton_client_t::try_client_mode() {
  boost::asio::local::stream_protocol::socket socket{io_};
  boost::system::error_code ec;
  socket.connect(get<0>(state_), ec);

  if (ec) {
    const bool first_instance =
        ec == boost::asio::error::connection_refused ||
        ec == boost::system::error_code{
                  ENOENT, boost::asio::error::get_system_category()};
    if (first_instance) {
      spdlog::info("The singleton server is not active ({}): {}", ec.value(),
                   ec.message());
      return false;
    } else {
      throw boost::system::system_error{ec, "singleton client connect"};
    }
  } else {
    spdlog::info("The singleton server is active");
    state_.emplace<singleton::client_t>(*this, std::move(socket));
    return true;
  }
}

// application_t::mode_t
void singleton_client_t::run() {
  get<singleton::client_t>(state_).start();
  io_.run();
}

// singleton::client_t::state_dumper_t
msgpack::sbuffer singleton_client_t::dump_state() {
  msgpack::sbuffer buf;
  msgpack::pack(buf, nvim_socket_);
  return buf;
}

} // namespace monitor::application
