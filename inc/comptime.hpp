#pragma once
#include "types.hpp"

namespace comptime {
  // gets us the first pixels row of sample directions during compile time.
  //
  // subsequent render iterations will simply scale this by
  // row and column index to find where to take samples
  consteval Vec3_256 init_ray_directions();

  consteval __m256i init_rseed_arr();

}; // namespace comptime
