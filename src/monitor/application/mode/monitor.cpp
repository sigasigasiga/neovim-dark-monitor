#include "monitor/application/mode/monitor.hpp"

#include "monitor/service/inventory.hpp"
#include "monitor/util/asio/endpoint.hpp"

namespace monitor::application::mode {

namespace {

auto convert_appearance(
    siga::dark_notify::dark_notify_t::appearance_t appearance) {
  return static_cast<service::appearance_t>(appearance);
}

auto make_socket(boost::asio::any_io_executor exec,
                 const std::string &str_endpoint) {
  if (auto ep = util::asio::make_endpoint(str_endpoint)) {
    boost::asio::generic::stream_protocol::socket ret{exec};
    ret.connect(*ep);
    return ret;
  } else {
    throw std::runtime_error{"Invalid neovim endpoint"};
  }
}

class scoped_callback_register_t : private util::scoped_t {
public:
  using dark_notify_t = siga::dark_notify::dark_notify_t;

public:
  scoped_callback_register_t(
      dark_notify_t &notifier,
      std::function<void(dark_notify_t::appearance_t)> cb)
      : notifier_{notifier} {
    notifier_.register_callback(std::move(cb));
  }

  ~scoped_callback_register_t() { notifier_.unregister_callback(); }

private:
  dark_notify_t &notifier_;
};

class scoped_notifier_stopper_t : private util::scoped_t {
public:
  scoped_notifier_stopper_t(siga::dark_notify::dark_notify_t &notifier)
      : notifier_{notifier} {}

  ~scoped_notifier_stopper_t() {
    // well, if this method call throws, we don't mind calling `std::terminate`,
    // because we are about to stop the process anyway
    notifier_.stop();
  }

private:
  siga::dark_notify::dark_notify_t &notifier_;
};

} // anonymous namespace

monitor_t::monitor_t(const std::string &singleton_endpoint,
                     const std::string &nvim_endpoint)
    : notifier_{siga::dark_notify::make_default_notifier()}, signal_set_{io_},
      inventory_{service::make_inventory(
          io_.get_executor(), singleton_endpoint,
          make_socket(io_.get_executor(), nvim_endpoint),
          *this, // service::request_handler_t::query_t
          *this, // service::neovim_t::delegate_t
          on_theme_change_)} {}

// application_t::mode_t
void monitor_t::run() {
  auto io_fut = std::async(std::launch::async, [this] {
    scoped_notifier_stopper_t guard{*notifier_};
    io_.run();
  });

  signal_set_.add(SIGINT);
  signal_set_.add(SIGTERM);
  signal_set_.async_wait(
      wrap(std::bind_front(&monitor_t::handle_signal, this)));

  boost::asio::post(io_, [this] { inventory_.reload(); });

  scoped_callback_register_t register_guard{
      *notifier_, [this](auto appearance) {
        boost::asio::post(io_, std::bind_front(std::ref(on_theme_change_),
                                               convert_appearance(appearance)));
      }};
  // `dark_notify_t::run` must be run on the main thread on macOS
  notifier_->run();

  io_fut.wait();
}

// service::request_handler_t::query_t
service::appearance_t monitor_t::query() {
  return convert_appearance(notifier_->query());
}

// service::neovim_t::delegate_t
void monitor_t::on_jobs_finished() {
  spdlog::info("All neovim jobs were finished");
  stop();
}

// private
void monitor_t::handle_signal(const boost::system::error_code &ec,
                              int signal_number) {
  if (ec) {
    throw boost::system::system_error{ec, "signal set"};
  }

  spdlog::info("Got signal {}", signal_number);
  stop();
}

void monitor_t::stop() {
  if (!inventory_.active()) {
    // This should never happen
    spdlog::warn("The inventory is already stopping");
    return;
  }

  inventory_.stop([this] {
    spdlog::info("The inventory has stopped");
    notifier_->stop();
    io_.stop();
  });
}

} // namespace monitor::application::mode
