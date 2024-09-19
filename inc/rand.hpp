#pragma once
#include "comptime.hpp"
#include "math.h"

class LCGRand {
public:
  [[nodiscard]] Vec3_256 random_unit_vec();

  [[nodiscard]] float rand_in_range(float min, float max);

  [[nodiscard]] __m256 rand_in_range_256(float min, float max);

private:
	// TODO BUG these used to be static thread_local. They probably still need to be?
  static inline thread_local __m256i rseed_vec = comptime::init_rseed_arr();
  static inline thread_local uint32_t rseed = 0;
  const __m256i r_a = _mm256_set1_epi32((uint32_t)11035152453u);
  const __m256i r_b = _mm256_set1_epi32(12345u);
  const __m256i rand_max_vec = _mm256_set1_epi32(RAND_MAX);
  static constexpr float rcp_rand_max = 1.f / RAND_MAX;

  [[nodiscard]] Vec3_256 rand_vec_in_cube();

  [[nodiscard]] __m256i lcg_rand_256();

  // scalar versions of rand generation
  [[nodiscard]] int lcg_rand();
};
