#pragma once

#include "monitor/util/scoped.hpp"
#include "monitor/util/signal_ref.hpp"

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
concept job = std::convertible_to<T *, job_t *>;

template <job T> class job_storage_t {
public:
  job_storage_t() = default;

public:
  template <job Job = T, typename... Args> Job &make_job(Args &&...args);
  auto range() const { return jobs_ | ranges::views::indirect; }
  signal_ref_t<> jobs_finished_sig() { return on_jobs_finished_; }

private:
  using job_list_t = std::list<std::unique_ptr<T>>;
  using job_handle_t = job_list_t::const_iterator;
  using job_empty_sig_t = boost::signals2::signal<void()>;

  class job_remove_t : public job_t::delegate_t {
  public:
    job_remove_t(job_list_t &jobs, job_empty_sig_t &empty_sig,
                 job_handle_t handle);

  private: // job_t::delegate_t
    void job_finished() final;

  private:
    job_list_t &jobs_;
    job_empty_sig_t &empty_sig_;
    job_handle_t handle_;
  };

private:
  job_list_t jobs_;
  job_empty_sig_t on_jobs_finished_;
};

template <job T>
template <job Job, typename... Args>
Job &job_storage_t<T>::make_job(Args &&...args) {
  static_assert(std::constructible_from<Job, std::unique_ptr<job_t::delegate_t>,
                                        Args &&...>);
  auto &job_ptr = jobs_.emplace_front(nullptr);
  auto job_remover =
      std::make_unique<job_remove_t>(jobs_, on_jobs_finished_, jobs_.begin());
  job_ptr = std::make_unique<Job>(std::move(job_remover),
                                  std::forward<Args>(args)...);
  return *job_ptr;
}

template <job T>
job_storage_t<T>::job_remove_t::job_remove_t(job_list_t &jobs,
                                             job_empty_sig_t &empty_sig,
                                             job_handle_t handle)
    : jobs_{jobs}, empty_sig_{empty_sig}, handle_{std::move(handle)} {}

template <job T> void job_storage_t<T>::job_remove_t::job_finished() {
  // After the call to `jobs_.erase(handle_)` we cannot access `jobs_` and
  // `empty_sig_` anymore, thus we save them here on the stack frame.
  // TODO: I guess I have to think about a bit more elegant solution
  auto &jobs = jobs_;
  auto &empty_sig = empty_sig_;

  jobs.erase(handle_);
  if (jobs.empty()) {
    empty_sig();
  }
}

} // namespace monitor::util
