#pragma once
#include "comptime.hpp"

class LCGRand {
public:
  [[nodiscard, gnu::always_inline]] inline Vec3_256 random_unit_vec() {
    Vec3_256 rand_vec = rand_vec_in_cube();
    rand_vec.normalize();
    return rand_vec;
  };

  [[nodiscard, gnu::always_inline]] inline float rand_in_range(const float min, const float max) {
    const float scale = static_cast<float>(lcg_rand()) * rcp_rand_max;
    const float f = min + scale * (max - min);
    return f;
  }

  [[nodiscard]] inline __m256 rand_in_range_256(float min, float max) {
    __m256i scale_i32 = lcg_rand_256();
    __m256 scale = _mm256_cvtepi32_ps(scale_i32);
    __m256 rcp_rand_max_vec = _mm256_broadcast_ss(&rcp_rand_max);
    scale *= rcp_rand_max_vec;

    __m256 min_vec = _mm256_broadcast_ss(&min);
    __m256 max_vec = _mm256_broadcast_ss(&max);
    __m256 range = max_vec - min_vec;

    return _mm256_fmadd_ps(scale, range, min_vec);
  }

private:
  static inline __m256i rseed_vec = comptime::init_rseed_arr();
  static inline uint32_t rseed = 0;
  const __m256i r_a = _mm256_set1_epi32(static_cast<int>(11035152453));
  const __m256i r_b = _mm256_set1_epi32(12345u);
  const __m256i rand_max_vec = _mm256_set1_epi32(RAND_MAX);
  static constexpr float rcp_rand_max = 1.f / static_cast<float>(RAND_MAX);

  [[nodiscard, gnu::always_inline]] inline Vec3_256 rand_vec_in_cube() {
    constexpr float min = -1.0;
    constexpr float max = 1.0;

    return Vec3_256{
        rand_in_range_256(min, max),
        rand_in_range_256(min, max),
        rand_in_range_256(min, max),
    };
  }

  [[nodiscard, gnu::always_inline]] inline __m256i lcg_rand_256() {
    rseed_vec = _mm256_mullo_epi32(rseed_vec, r_a);
    rseed_vec = _mm256_add_epi32(rseed_vec, r_b);
    rseed_vec = _mm256_and_si256(rseed_vec, rand_max_vec);
    return rseed_vec;
  };

  // scalar versions of rand generation
  [[nodiscard, gnu::always_inline]] inline int lcg_rand() {
    return rseed = (rseed * 1103515245 + 12345) & RAND_MAX;
  }
};
