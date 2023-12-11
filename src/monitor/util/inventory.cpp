#include "monitor/util/inventory.hpp"

namespace monitor::util {

namespace {

class stop_callback_t {
public:
  stop_callback_t(boost::asio::any_io_executor exec,
                  std::function<void()> &callback, std::uintmax_t &count)
      : exec_{std::move(exec)}, callback_{&callback}, count_{&count} {}

public:
  void operator()() {
    if (--(*count_) == 0) {
      boost::asio::post(exec_, std::move(*callback_));
    }
  }

private:
  boost::asio::any_io_executor exec_;
  std::function<void()> *callback_;
  std::uintmax_t *count_;
};

} // anonymous namespace

inventory_t::inventory_t(boost::asio::any_io_executor exec,
                         std::list<std::unique_ptr<service_t>> services)
    : exec_{std::move(exec)}, state_{std::in_place_type<service_list_t>,
                                     std::move(services)} {}

bool inventory_t::active() const {
  return std::holds_alternative<service_list_t>(state_);
}

void inventory_t::reload() {
  ranges::for_each(std::get<service_list_t>(state_), &service_t::reload);
  spdlog::info("The inventory was reloaded");
}

void inventory_t::stop(std::function<void()> callback) {
  auto services = std::move(std::get<service_list_t>(state_));
  state_.emplace<stop_t>(exec_, std::move(services), std::move(callback));
}

// stop_t
inventory_t::stop_t::stop_t(boost::asio::any_io_executor exec,
                            service_list_t &&services,
                            std::function<void()> callback)
    : exec_{std::move(exec)},
      reversed_services_{std::move(services)}, callback_{std::move(callback)} {
  reversed_services_.reverse();
  stop();
}

void inventory_t::stop_t::stop() {
  auto stoppable_it =
      std::ranges::find_if(reversed_services_, [](const auto &svc_ptr) {
        return !!dynamic_cast<const stoppable_service_t *>(svc_ptr.get());
      });

  reversed_services_.erase(reversed_services_.begin(), stoppable_it);
  if (stoppable_it == reversed_services_.end()) {
    boost::asio::post(exec_, std::move(callback_));
  } else {
    auto &stoppable = static_cast<stoppable_service_t &>(**stoppable_it);
    stoppable.stop([this, stoppable_it] {
      reversed_services_.erase(stoppable_it);
      stop();
    });
  }
}

} // namespace monitor::util
