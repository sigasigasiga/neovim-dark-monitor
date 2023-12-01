#pragma once

#include "monitor/dark_notifier/dark_notifier.hpp"
#include "monitor/neovim/client.hpp"
#include "monitor/util/functional.hpp"
#include "monitor/util/service.hpp"

namespace monitor::service {

class dark_notifier_t : public util::service_t {
public:
  dark_notifier_t(boost::asio::any_io_executor exec,
                  const util::job_storage_t<neovim::client_t> &neovim_jobs);

private: // util::service_t
  void reload() final;

private: // dark_notifier::dark_notifier_t slots
  void
  on_theme_change(siga::dark_notify::dark_notify_t::appearance_t appearance);

private:
  util::first_flag_t first_;
  const util::job_storage_t<neovim::client_t> &neovim_jobs_;
  dark_notifier::dark_notifier_t dark_notifier_;
  boost::signals2::scoped_connection connection_;
};

} // namespace monitor::service
