#pragma once

#include "monitor/util/scoped.hpp"

namespace monitor::util {

class callback_wrapper_t : private scoped_t {
public:
  callback_wrapper_t() : cancelled_{std::make_shared<bool>(false)} {}
  ~callback_wrapper_t() { cancel(); }

private:
  template <typename F> class wrap_t;

public:
  template <typename F> wrap_t<F> wrap(F func) const {
    return wrap_t{std::move(func), cancelled_};
  }

  void cancel() noexcept {
    assert(cancelled_);
    *cancelled_ = true;
  }

private:
  std::shared_ptr<bool> cancelled_;
};

template <typename F> class callback_wrapper_t::wrap_t {
public:
  wrap_t(F func, std::shared_ptr<const bool> cancelled)
      : func_{std::move(func)}, cancelled_{std::move(cancelled)} {}

public:
  template <typename... Args> decltype(auto) operator()(Args &&...args) {
    if (*cancelled_) {
      return std::invoke_result_t<F, Args &&...>();
    } else {
      return std::invoke(func_, std::forward<Args>(args)...);
    }
  }

private:
  F func_;
  std::shared_ptr<const bool> cancelled_;
};

} // namespace monitor::util
