#pragma once

namespace monitor::util {

template <typename T, typename... Ts>
concept one_of = (std::same_as<T, Ts> || ...);

} // namespace monitor::util
