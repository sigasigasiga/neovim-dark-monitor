#pragma once

#include "monitor/neovim/client.hpp"
#include "monitor/singleton/server.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/service.hpp"
#include "monitor/util/signal_ref.hpp"

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

public:
  neovim_t(delegate_t &delegate, boost::asio::any_io_executor exec,
           boost::asio::generic::stream_protocol::socket current_client);

  const auto &clients() const { return clients_; }
  util::signal_ref_t<neovim::client_t &> new_client_sig() {
    return new_client_sig_;
  }

private: // singleton::server_t::msg_handler_t
  void on_client_msg(msgpack::object_handle handle) final;

private: // util::job_storage_t<neovim::client_t> signal handlers
  void on_jobs_finished();

private:
  delegate_t &delegate_;
  boost::asio::any_io_executor exec_;
  util::job_storage_t<neovim::client_t> clients_;
  boost::signals2::signal<void(neovim::client_t &)> new_client_sig_;
  util::signal_ref_t<>::connection_t on_jobs_finished_;
};

} // namespace monitor::service
