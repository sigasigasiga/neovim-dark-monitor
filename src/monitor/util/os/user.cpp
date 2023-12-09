#include "monitor/util/os/user.hpp"

#ifdef _WIN32

#include <Lmcons.h>

#else // _WIN32

#include <unistd.h>

#endif // _WIN32

namespace monitor::util::os {

std::string get_user_id() {
#ifdef _WIN32

  char buf[UNLEN];
  DWORD sz = ranges::size(buf);
  if (::GetUserNameA(buf, &sz) == 0) {
    throw std::system_error(::GetLastError(), std::system_category(),
                            "get_user_id");
  } else {
    return std::string(buf, sz);
  }

#else // _WIN32

  return std::to_string(::geteuid());

#endif // _WIN32
}

} // namespace monitor::util::os
