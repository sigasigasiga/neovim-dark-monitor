#pragma once

#include "monitor/util/service.hpp"

namespace monitor::util {

class inventory_t {
public:
  inventory_t(boost::asio::any_io_executor exec,
              std::list<std::unique_ptr<service_t>> services);

public:
  void reload();
  void stop(std::function<void()> callback);

private:
  boost::asio::any_io_executor exec_;
  std::list<std::unique_ptr<service_t>> services_;

  std::uintmax_t stop_count_ = 0;
  std::function<void()> stop_callback_;
};

} // namespace monitor::util
