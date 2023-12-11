#pragma once

#include "monitor/util/service.hpp"

namespace monitor::util {

class inventory_t {
public:
  inventory_t(boost::asio::any_io_executor exec,
              std::list<std::unique_ptr<service_t>> services);

public:
  bool active() const;
  void reload();
  void stop(std::function<void()> callback);

private:
  using service_list_t = std::list<std::unique_ptr<service_t>>;

  class stop_t {
  public:
    stop_t(boost::asio::any_io_executor exec, service_list_t &&services,
           std::function<void()> callback);

  private:
    void stop();

  private:
    boost::asio::any_io_executor exec_;
    service_list_t reversed_services_;
    std::function<void()> callback_;
  };

private:
  boost::asio::any_io_executor exec_;
  std::variant<service_list_t, stop_t> state_;
};

} // namespace monitor::util
