#pragma once

#include "monitor/neovim/client.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/functional.hpp"
#include "monitor/util/service.hpp"
#include "monitor/util/signal_ref.hpp"

namespace monitor::service {

class monitor_t : public util::service_t, private util::callback_wrapper_t {
public:
  class notifier_t {
  public:
    enum class appearance_t { unknown, light, dark };
    using appearance_sig_t = util::signal_ref_t<appearance_t>;

  public:
    virtual ~notifier_t() = default;

  public:
    virtual appearance_t query() = 0;
    virtual appearance_sig_t signal() = 0;
  };

  using neovim_client_sig_t = util::signal_ref_t<neovim::client_t &>;

public:
  monitor_t(notifier_t &notifier,
            const util::job_storage_t<neovim::client_t> &neovim_clients,
            neovim_client_sig_t nvim_client_sig);

private: // util::service_t
  void reload() final;

private: // notifier_t signal handlers
  void on_appearance(notifier_t::appearance_t appearance);

private: // neovim_client_sig_t signal handlers
  void on_neovim_client(neovim::client_t &client);

private:
  util::first_flag_t first_;
  notifier_t &notifier_;
  const util::job_storage_t<neovim::client_t> &neovim_clients_;
  notifier_t::appearance_sig_t::connection_t appearance_connection_;
  neovim_client_sig_t::connection_t neovim_connection_;
};

} // namespace monitor::service
