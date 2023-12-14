#include "monitor/singleton/server.hpp"

#include "monitor/util/error.hpp"

namespace monitor::singleton {

server_t::server_t(msg_handler_t &msg_handler,
                   boost::asio::any_io_executor exec,
                   boost::asio::local::stream_protocol::endpoint ep)
    : msg_handler_{msg_handler}, exec_{std::move(exec)}, acceptor_{exec_} {
  std::filesystem::remove(ep.path(), util::ignore_std_error);

  acceptor_.open(ep.protocol());
  acceptor_.bind(ep);
  acceptor_.listen(boost::asio::socket_base::max_listen_connections);
}

void server_t::start() {
  spdlog::info("Starting the singleton server at {} ",
               acceptor_.local_endpoint().path());
  async_accept();
}

void server_t::async_accept() {
  acceptor_.async_accept(wrap(std::bind_front(&server_t::handle_accept, this)));
}

void server_t::handle_accept(
    const boost::system::error_code &ec,
    boost::asio::local::stream_protocol::socket client_socket) {
  if (ec) {
    throw boost::system::system_error{ec, "Singleton server accept"};
  }

  jobs_.make_job(msg_handler_, std::move(client_socket));
  async_accept();
}

} // namespace monitor::singleton
