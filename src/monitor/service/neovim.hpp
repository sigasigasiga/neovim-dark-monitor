#pragma once

#include "monitor/rpc/client.hpp"
#include "monitor/singleton/server.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/functional.hpp"
#include "monitor/util/job.hpp"
#include "monitor/util/service.hpp"

namespace monitor::service {

class neovim_t : public util::service_t,
                 public singleton::server_t::msg_handler_t,
                 private util::callback_wrapper_t {
public:
  class delegate_t {
  public:
    virtual ~delegate_t() = default;
    virtual void on_jobs_finished() = 0;
  };

  class client_init_t {
  public:
    virtual ~client_init_t() = default;
    virtual void on_new_client(rpc::client_t &client) = 0;
  };

public:
  neovim_t(delegate_t &delegate, client_init_t &client_init,
           rpc::client_t::request_handler_t &request_handler,
           boost::asio::any_io_executor exec,
           boost::asio::generic::stream_protocol::socket current_client);

public:
  auto clients() const {
    return clients_.range() | ranges::views::transform(&rpc_client_job_t::get);
  }

private: // util::service_t
  void reload() final;

private: // singleton::server_t::msg_handler_t
  void on_client_msg(msgpack::object_handle handle) final;

private: // util::job_storage_t<neovim::client_t> signal handlers
  void on_jobs_finished();

private:
  class rpc_client_job_t : public util::job_t,
                           public rpc::client_t::delegate_t {
  public:
    rpc_client_job_t(std::unique_ptr<job_t::delegate_t> job_delegate,
                     rpc::client_t::request_handler_t &request_handler,
                     boost::asio::generic::stream_protocol::socket socket);

  public:
    rpc::client_t &get() { return client_; }

  private: // rpc::client_t::delegate_t
    void on_client_error(std::exception_ptr ep) final;

  private:
    rpc::client_t client_;
  };

private:
  delegate_t &delegate_;
  client_init_t &client_init_;
  rpc::client_t::request_handler_t &request_handler_;

  boost::asio::any_io_executor exec_;
  util::job_storage_t<rpc_client_job_t> clients_;
  util::signal_ref_t<>::connection_t on_jobs_finished_;

  util::first_flag_t first_;
};

} // namespace monitor::service
