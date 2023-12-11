#pragma once

#include <siga/dark_notify/dark_notify.hpp>

#include "monitor/application/application.hpp"
#include "monitor/service/monitor.hpp"
#include "monitor/util/inventory.hpp"

namespace monitor::application::mode {

class monitor_t : public application_t::mode_t,
                  public service::monitor_t::notifier_t,
                  private util::callback_wrapper_t,
                  private util::scoped_t {
public:
  monitor_t(const std::string &singleton_endpoint,
            const std::string &nvim_endpoint);

private: // application_t::mode_t
  void run() final;

private: // service::monitor_t::notifier_t,
  appearance_t query() final;
  appearance_sig_t signal() final;

private:
  void handle_signal(const boost::system::error_code &ec, int signal_number);

private:
  boost::asio::io_context io_;
  std::unique_ptr<siga::dark_notify::dark_notify_t> notifier_;

  boost::asio::signal_set signal_set_;

  boost::signals2::signal<void(appearance_t)> on_theme_change_;
  util::inventory_t inventory_;
};

} // namespace monitor::application::mode
