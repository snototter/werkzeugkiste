#include <werkzeugkiste/timing/stopwatch.h>

namespace werkzeugkiste {
namespace timing {

// Explicit template instantiation
template class stop_watch<std::chrono::steady_clock>;

}  // namespace timing
}  // namespace werkzeugkiste
