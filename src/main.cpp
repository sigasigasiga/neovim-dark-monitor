#include <unistd.h>

#include <siga/dark_notify/dark_notify.hpp>

#include "monitor/service/inventory.hpp"
#include "monitor/service/neovim.hpp"
#include "monitor/singleton/client.hpp"
#include "monitor/util/error.hpp"
#include "monitor/util/inventory.hpp"

namespace monitor {

namespace {

std::string make_singleton_socket_filename() {
  return fmt::format("neovim-dark-monitor.{}.sock", ::geteuid());
}

std::filesystem::path make_runtime_path() {
  static const std::filesystem::path default_server_socket_path = "/tmp";

  const char *nvim_env = std::getenv("XDG_RUNTIME_DIR");
  if (nvim_env == nullptr) {
    spdlog::warn("$XDG_RUNTIME_DIR is not set, using {} instead",
                 default_server_socket_path.string());
    return default_server_socket_path;
  } else {
    return nvim_env;
  }
}

std::filesystem::path make_singleton_socket_path() {
  return make_runtime_path() / make_singleton_socket_filename();
}

std::string get_nvim_socket() {
  const char *nvim_socket_path = std::getenv("NVIM");
  if (nvim_socket_path) {
    spdlog::debug("Using NVIM={}", nvim_socket_path);
    return nvim_socket_path;
  } else {
    throw util::error_t::not_nvim_job;
  }
}

service::neovim_t::dark_notifier_t::appearance_t
convert_appearance(siga::dark_notify::dark_notify_t::appearance_t appearance) {
  return static_cast<service::neovim_t::dark_notifier_t::appearance_t>(
      appearance);
}

boost::program_options::variables_map parse_command_line(int argc,
                                                         const char *argv[]) {
  namespace po = boost::program_options;

  po::options_description desc{"neovim-dark-monitor options"};
  desc.add_options()                                                         //
      ("help", "produce help message")                                       //
      ("dark-theme", po::value<std::string>(), "the dark theme to be set")   //
      ("light-theme", po::value<std::string>(), "the light theme to be set") //
      ;

  po::variables_map ret;

  po::store(po::parse_command_line(argc, argv, desc), ret);
  po::notify(ret);

  return ret;
}

class application_t : public singleton::client_t::state_dumper_t,
                      public service::neovim_t::dark_notifier_t {
public:
  application_t(int argc, const char *argv[])
      : notifier_{siga::dark_notify::make_default_notifier()},
        cmd_line_{parse_command_line(argc, argv)}, nvim_socket_path_{
                                                       get_nvim_socket()} {}

public:
  void run() {
    make_state();

    auto io_fut = std::async(std::launch::async, [this] { io_.run(); });

    auto theme_cb = [this](auto appearance) {
      boost::asio::post(io_, std::bind_front(std::ref(on_theme_change_),
                                             convert_appearance(appearance)));
    };

    theme_cb(notifier_->query());
    notifier_->register_callback(theme_cb);
    notifier_->run();

    io_fut.wait();
  }

private: // singleton::client_t::state_dumper_t
  msgpack::sbuffer dump_state() final {
    msgpack::sbuffer buf;
    msgpack::pack(buf, nvim_socket_path_);
    return buf;
  }

private: // service::neovim_t::dark_notifier_t
  appearance_t query() final { return convert_appearance(notifier_->query()); }

  boost::signals2::scoped_connection
  subscribe(boost::signals2::slot<void(appearance_t)> slot) final {
    return on_theme_change_.connect(std::move(slot));
  }

private:
  void make_state() {
    boost::asio::local::stream_protocol::socket client_mode_socket{io_};
    boost::asio::local::stream_protocol::endpoint singleton_endpoint{
        make_singleton_socket_path().string()};

    boost::system::error_code ec;
    client_mode_socket.connect(singleton_endpoint, ec);
    if (ec == boost::asio::error::connection_refused ||
        ec == boost::system::error_code{
                  ENOENT, boost::asio::error::get_system_category()}) {
      spdlog::info("Singleton server is not active ({}): {}", ec.value(),
                   ec.message());

      boost::asio::local::stream_protocol::endpoint nvim_endpoint{
          nvim_socket_path_};
      boost::asio::local::stream_protocol::socket nvim_socket{io_};
      nvim_socket.connect(nvim_endpoint);

      auto &inv = state_.emplace<util::inventory_t>(service::make_inventory(
          io_.get_executor(), std::move(singleton_endpoint),
          std::move(nvim_socket), *this));

      boost::asio::post(io_, [&inv] { inv.reload(); });
    } else if (ec) {
      throw boost::system::system_error{ec,
                                        "Cannot connect to the server socket"};
    } else {
      spdlog::info("Starting the client at {}", singleton_endpoint.path());
      state_.emplace<singleton::client_t>(*this, std::move(client_mode_socket));
    }
  }

private:
  boost::asio::io_context io_;
  const std::unique_ptr<siga::dark_notify::dark_notify_t> notifier_;

  boost::program_options::variables_map cmd_line_;
  std::variant<std::monostate, singleton::client_t, util::inventory_t> state_;
  const std::string nvim_socket_path_;

  boost::signals2::signal<void(appearance_t)> on_theme_change_;
};

} // anonymous namespace

} // namespace monitor

int main(int argc, const char *argv[]) try {
  monitor::application_t app{argc, argv};
  app.run();
} catch (monitor::util::error_t error) {
  spdlog::error(monitor::util::error_to_string(error));
  return static_cast<int>(error);
} catch (const std::exception &ex) {
  spdlog::error(ex.what());
  return static_cast<int>(monitor::util::error_t::unhandled_exception);
} catch (...) {
  spdlog::error("Unexpected error");
  return static_cast<int>(monitor::util::error_t::unexpected_error);
}
