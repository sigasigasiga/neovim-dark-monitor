#pragma once

#include "monitor/rpc/client.hpp"

namespace monitor::neovim {

template <typename T>
concept method = requires(T params, msgpack::packer<msgpack::sbuffer> packer) {
                   { T::method_name } -> std::convertible_to<std::string_view>;
                   typename T::response_result_t;
                   packer.pack(params);
                 };

template <method Method>
using method_response_t =
    tl::expected<typename Method::response_result_t, msgpack::object_handle>;

template <typename F, typename Method>
concept method_callback =
    method<Method> &&
    requires() { requires std::is_invocable_v<F, method_response_t<Method>>; };

template <method Method, method_callback<Method> Cb>
void send(rpc::client_t &client, const Method &params, Cb cb) {
  class converting_cb_t {
  public:
    converting_cb_t(Cb cb) : cb_{std::move(cb)} {}

  public:
    void operator()(rpc::client_t::result_t result) {
      auto transformed_result =
          std::move(result)
              .transform(&msgpack::object_handle::get)
              .transform(
                  &msgpack::object::as<typename Method::response_result_t>);

      std::invoke(cb_, std::move(transformed_result));
    }

  private:
    Cb cb_;
  };

  msgpack::zone zone;
  msgpack::object params_object{params, zone};

  client.send_request(Method::method_name, params_object,
                      converting_cb_t{std::move(cb)});
}

} // namespace monitor::neovim
