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
    : exec_{std::move(exec)}, services_{std::move(services)} {}

void inventory_t::reload() { ranges::for_each(services_, &service_t::reload); }

void inventory_t::stop(std::function<void()> callback) {
  assert(!stop_callback_);
  assert(stop_count_ == 0);

  stop_callback_ = std::move(callback);

  auto count = std::make_shared<std::uintmax_t>();
  for (const auto &service : services_) {
    if (auto stoppable = dynamic_cast<stoppable_service_t *>(service.get())) {
      ++(*count);
      stoppable->stop(stop_callback_t{exec_, callback, stop_count_});
    }
  }
}

} // namespace monitor::util
