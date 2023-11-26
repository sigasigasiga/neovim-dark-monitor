#include <unistd.h>

#include <boost/program_options.hpp>

#include "monitor/service/inventory.hpp"
#include "monitor/singleton/client.hpp"
#include "monitor/util/error.hpp"
#include "monitor/util/inventory.hpp"

namespace monitor {

std::string make_socket_filename() {
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

std::filesystem::path make_socket_path() {
  return make_runtime_path() / make_socket_filename();
}

std::string get_nvim_socket() {
  char *nvim_socket_path = std::getenv("NVIM");
  if (nvim_socket_path) {
    return nvim_socket_path;
  } else {
    throw util::error_t::not_nvim_job;
  }
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

class application_t : public singleton::client_t::state_dumper_t {
public:
  application_t(int argc, const char *argv[])
      : cmd_line_{parse_command_line(argc, argv)}, nvim_socket_{
                                                       get_nvim_socket()} {}

public:
  void run() {
    boost::asio::local::stream_protocol::socket client_mode_socket{io_};
    boost::asio::local::stream_protocol::endpoint singleton_endpoint{
        make_socket_path().string()};

    boost::system::error_code ec;
    client_mode_socket.connect(singleton_endpoint, ec);
    if (ec == boost::asio::error::connection_refused ||
        ec == boost::system::error_code{
                  ENOENT, boost::asio::error::get_system_category()}) {
      spdlog::info("Singleton server is not active ({}): {}", ec.value(),
                   ec.message());
      auto &inv = state_.emplace<util::inventory_t>(service::make_inventory(
          io_.get_executor(), std::move(singleton_endpoint), nvim_socket_));

      inv.reload(); // TODO: when do we reload?
    } else if (ec) {
      throw boost::system::system_error{ec,
                                        "Cannot connect to the server socket"};
    } else {
      spdlog::info("Starting the client at {}", singleton_endpoint.path());
      state_.emplace<singleton::client_t>(*this, std::move(client_mode_socket));
    }

    io_.run();
  }

private: // singleton::client_t::state_dumper_t
  msgpack::sbuffer dump_state() final {
    msgpack::sbuffer buf;
    msgpack::pack(buf, nvim_socket_);
    return buf;
  }

private:
  boost::asio::io_context io_;
  boost::program_options::variables_map cmd_line_;
  std::variant<std::monostate, singleton::client_t, util::inventory_t> state_;
  const std::string nvim_socket_;
};

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
