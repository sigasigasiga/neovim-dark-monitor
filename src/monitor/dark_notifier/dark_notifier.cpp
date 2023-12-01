#include "monitor/dark_notifier/dark_notifier.hpp"

namespace monitor::dark_notifier {

dark_notifier_t::dark_notifier_t(boost::asio::any_io_executor exec)
    : strand_{exec}, notifier_{siga::dark_notify::make_default_notifier()} {}

dark_notifier_t::~dark_notifier_t() {
  notifier_->stop();
  notifier_->unregister_callback();
  notifier_thread_.join();
}

void dark_notifier_t::start() {
  boost::asio::post(strand_, wrap(std::bind_front(std::ref(on_theme_change_),
                                                  notifier_->query())));

  notifier_->register_callback([this](auto appearance) {
    boost::asio::post(std::bind_front(std::ref(on_theme_change_), appearance));
  });

  notifier_thread_ =
      std::thread{&siga::dark_notify::dark_notify_t::run, std::ref(*notifier_)};
}

} // namespace monitor::dark_notifier
