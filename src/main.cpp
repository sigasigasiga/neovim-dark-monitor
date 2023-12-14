#include "monitor/application/application.hpp"
#include "monitor/util/error.hpp"

int main(int argc, const char *argv[]) try {
  monitor::application::application_t app{argc, argv};
  return app.run();
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
