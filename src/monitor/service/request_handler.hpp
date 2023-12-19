#pragma once

#include "monitor/util/service.hpp"

#include "monitor/service/appearance.hpp"
#include "monitor/service/neovim.hpp"

namespace monitor::service {

class request_handler_t : public util::service_t,
                          public neovim_t::client_init_t,
                          public rpc::client_t::request_handler_t {
public:
  class query_t {
  public:
    virtual ~query_t() = default;
    virtual appearance_t query() = 0;
  };

public:
  request_handler_t(query_t &query);

private: // neovim_t::client_init_t
  void on_new_client(rpc::client_t &client) final;

private: // rpc::client_t::request_handler_t
  void on_request(std::string_view method, msgpack::object_handle data,
                  rpc::client_t::response_cb_t send_response) final;

private:
  query_t &query_;
};

} // namespace monitor::service
