#pragma once

#include "monitor/util/scoped.hpp"

namespace monitor::util {

class job_t : private scoped_t {
public:
  class delegate_t {
  public:
    virtual ~delegate_t() = default;
    virtual void job_finished() = 0;
  };

public:
  job_t(std::unique_ptr<delegate_t> delegate)
      : delegate_{std::move(delegate)} {}

protected:
  virtual ~job_t() = default;

protected:
  void job_finished() { delegate_->job_finished(); }

private:
  const std::unique_ptr<delegate_t> delegate_;
};

template <typename T>
  requires std::convertible_to<T *, job_t *>
class job_storage_t {
public:
  template <typename Job = T, typename... Args> Job &make_job(Args &&...args) {
    static_assert(
        std::constructible_from<Job, std::unique_ptr<job_t::delegate_t>,
                                Args &&...>);
    auto &job_ptr = jobs_.emplace_front(nullptr);
    auto job_remover = std::make_unique<job_remove_t>(jobs_, jobs_.begin());
    job_ptr = std::make_unique<Job>(std::move(job_remover),
                                    std::forward<Args>(args)...);
    return *job_ptr;
  }

public:
private:
  using job_list_t = std::list<std::unique_ptr<T>>;
  using job_handle_t = job_list_t::const_iterator;

  class job_remove_t : public job_t::delegate_t {
  public:
    job_remove_t(job_list_t &jobs, job_handle_t handle)
        : jobs_{jobs}, handle_{handle} {}

  private: // job_t::delegate_t
    void job_finished() final { jobs_.erase(handle_); }

  private:
    job_list_t &jobs_;
    job_handle_t handle_;
  };

private:
  std::list<std::unique_ptr<T>> jobs_;
};

} // namespace monitor::util
