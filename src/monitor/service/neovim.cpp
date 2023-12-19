#include "monitor/service/neovim.hpp"

#include "monitor/util/asio/endpoint.hpp"
#include "monitor/util/asio/error.hpp"
#include "monitor/util/error.hpp"

namespace monitor::service {

neovim_t::neovim_t(delegate_t &delegate, client_init_t &client_init,
                   rpc::client_t::request_handler_t &request_handler,
                   boost::asio::any_io_executor exec,
                   boost::asio::generic::stream_protocol::socket current_client)
    : delegate_{delegate}, client_init_{client_init},
      request_handler_{request_handler}, exec_{std::move(exec)},
      on_jobs_finished_{clients_.jobs_finished_sig().subscribe(
          std::bind_front(&neovim_t::on_jobs_finished, this))} {
  clients_.make_job(request_handler_, std::move(current_client));
}

// util::service_t
void neovim_t::reload() {
  if (first_.get()) {
    for (auto &client : clients()) {
      client_init_.on_new_client(client);
    }
  }
}

// singleton::server_t::msg_handler_t
void neovim_t::on_client_msg(msgpack::object_handle handle) {
  auto socket = handle.get().as<std::string>();
  auto endpoint = util::asio::make_endpoint(socket);
  if (!endpoint) {
    spdlog::error(
        "Got an invalid neovim endpoint path from the singleton client");
    return;
  }

  auto socket_ptr =
      std::make_unique<boost::asio::generic::stream_protocol::socket>(exec_);
  auto &socket_ref = *socket_ptr;

  auto cb = [this, socket_ptr = std::move(socket_ptr)](
                const boost::system::error_code &ec) {
    if (ec) {
      spdlog::error("Cannot connect to the socket: {} ({})", ec.message(),
                    ec.value());
    } else {
      auto &client =
          clients_.make_job(request_handler_, std::move(*socket_ptr));
      client_init_.on_new_client(client.get());
    }
  };

  socket_ref.async_connect(*endpoint, wrap(std::move(cb)));
}

// util::job_storage_t<neovim::client_t> signal handlers
void neovim_t::on_jobs_finished() {
  boost::asio::post(exec_, std::bind_front(&delegate_t::on_jobs_finished,
                                           std::ref(delegate_)));
}

// rpc_client_job_t
neovim_t::rpc_client_job_t::rpc_client_job_t(
    std::unique_ptr<job_t::delegate_t> job_delegate,
    rpc::client_t::request_handler_t &request_handler,
    boost::asio::generic::stream_protocol::socket socket)
    : job_t{std::move(job_delegate)}, client_{*this, request_handler,
                                              std::move(socket)} {}

// rpc::client_t::delegate_t
void neovim_t::rpc_client_job_t::on_client_error(std::exception_ptr ep) {
  assert(ep);

  if (util::asio::is_disconnected(ep)) {
    spdlog::info("The client was disconnected");
  } else {
    spdlog::error("The client has encountered an error: {}", util::what(ep));
  }

  job_finished();
}

} // namespace monitor::service
