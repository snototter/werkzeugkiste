#include <cmath>

#include <werkzeugkiste/geometry/vector.h>

namespace werkzeugkiste::geometry {

// Explicit instantiation:
template class Vec<float, 2>;
template class Vec<float, 3>;
template class Vec<float, 4>;

template class Vec<double, 2>;
template class Vec<double, 3>;
template class Vec<double, 4>;

template class Vec<int32_t, 2>;
template class Vec<int32_t, 3>;
template class Vec<int32_t, 4>;

} // namespace werkzeugkiste::geometry
