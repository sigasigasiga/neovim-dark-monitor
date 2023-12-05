#pragma once

#include <boost/signals2/signal.hpp>

#include "monitor/neovim/client.hpp"
#include "monitor/singleton/server.hpp"
#include "monitor/util/callback_wrapper.hpp"
#include "monitor/util/service.hpp"

namespace monitor::service {

class neovim_t : public util::service_t,
                 public singleton::server_t::msg_handler_t,
                 private util::callback_wrapper_t {
public:
  class dark_notifier_t {
  public:
    enum class appearance_t { unknown, light, dark };

  public:
    virtual ~dark_notifier_t() = default;

  public:
    virtual appearance_t query() = 0;
    virtual boost::signals2::scoped_connection
    subscribe(boost::signals2::slot<void(appearance_t)> slot) = 0;
  };

public:
  neovim_t(boost::asio::any_io_executor exec,
           boost::asio::generic::stream_protocol::socket current_client,
           dark_notifier_t &notifier);

private: // singleton::server_t::msg_handler_t
  void on_client_msg(msgpack::object_handle handle) final;

private: // dark_notifier_t signal slots
  void on_theme_change(dark_notifier_t::appearance_t appearance);

private:
  boost::asio::any_io_executor exec_;
  dark_notifier_t &notifier_;
  boost::signals2::scoped_connection dark_notifier_connection_;

  util::job_storage_t<neovim::client_t> clients_;
};

} // namespace monitor::service
