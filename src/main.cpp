#include "monitor/application/application.hpp"

int main(int argc, const char *argv[]) {
  monitor::application::application_t app{argc, argv};
  return app.run();
}
