#pragma once

#include <boost/signals2/signal.hpp>

#include <siga/dark_notify/dark_notify.hpp>

#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/scoped.hpp"

namespace monitor::dark_notifier {

class dark_notifier_t : private util::callback_wrapper_t,
                        private util::scoped_t {
public:
  dark_notifier_t(boost::asio::any_io_executor exec);
  ~dark_notifier_t();

public:
  template <typename F> boost::signals2::scoped_connection subscribe(F &&func) {
    return on_theme_change_.connect(std::forward<F>(func));
  }

  void start();

private:
  boost::asio::strand<boost::asio::any_io_executor> strand_;
  std::unique_ptr<siga::dark_notify::dark_notify_t> notifier_;
  std::thread notifier_thread_;
  boost::signals2::signal<void(siga::dark_notify::dark_notify_t::appearance_t)>
      on_theme_change_;
};

} // namespace monitor::dark_notifier
