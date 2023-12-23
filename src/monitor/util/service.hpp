#pragma once

#include "monitor/util/scoped.hpp"

namespace monitor::util {

class service_t : private scoped_t {
public:
  virtual ~service_t() = default;

public:
  virtual void reload() {}
};

class stoppable_service_t : public service_t {
public:
  virtual void stop(std::function<void()> callback) = 0;
};

} // namespace monitor::util
