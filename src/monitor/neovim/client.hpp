#pragma once

#include "monitor/util/job.hpp"

namespace monitor::neovim {

class client_t : public util::job_t {
public:
  client_t(std::unique_ptr<job_t::delegate_t> job_delegate,
           boost::asio::generic::stream_protocol::socket socket);

private:
  boost::asio::generic::stream_protocol::socket socket_;
};

} // namespace monitor::neovim
