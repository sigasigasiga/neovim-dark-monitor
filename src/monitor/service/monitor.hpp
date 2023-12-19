#pragma once

#include "monitor/rpc/client.hpp"
#include "monitor/service/appearance.hpp"
#include "monitor/util/service.hpp"
#include "monitor/util/signal_ref.hpp"

namespace monitor::service {

class monitor_t : public util::service_t {
public:
  using appearance_signal_t = util::signal_ref_t<appearance_t>;

public:
  monitor_t(appearance_signal_t sig,
            ranges::any_view<rpc::client_t &> neovim_clients);

private: // appearance_signal_t signal handler
  void on_appearance(appearance_t appearance);

private:
  ranges::any_view<rpc::client_t &> neovim_clients_;
  appearance_signal_t::connection_t appearance_connection_;
};

} // namespace monitor::service
