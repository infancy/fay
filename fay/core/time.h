#pragma once

#include <chrono>

namespace fay
{

// unix stamp
inline auto now() { return std::chrono::high_resolution_clock::now().time_since_epoch().count(); }

} // namespace fay