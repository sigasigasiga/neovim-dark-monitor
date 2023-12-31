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

class inventory_builder_t {
public:
  inventory_builder_t(boost::asio::any_io_executor exec)
      : exec_{std::move(exec)} {}

public:
  template <typename T, typename... Args>
  [[nodiscard]] T &add_service(Args &&...args) {
    static_assert(std::convertible_to<T *, service_t *>);
    auto svc_ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto &svc_ref = *svc_ptr;
    services_.emplace_back(std::move(svc_ptr));
    return svc_ref;
  }

  inventory_t make_inventory() {
    assert(!services_.empty());
    return inventory_t{std::move(exec_), std::move(services_)};
  }

private:
  boost::asio::any_io_executor exec_;
  std::list<std::unique_ptr<service_t>> services_;
};

} // namespace monitor::util
