#pragma once

#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/scoped.hpp"

namespace monitor::application {

class application_t : private util::scoped_t {
public:
  class mode_t {
  public:
    virtual ~mode_t() = default;
    virtual void run() = 0;
  };

public:
  application_t(int argc, const char *argv[]);

public:
  int run();

private:
  const boost::program_options::options_description options_description_;
  const boost::program_options::variables_map cmd_line_;

  std::unique_ptr<mode_t> state_;
};

} // namespace monitor::application
