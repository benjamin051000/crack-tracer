#include "rand.hpp"
#include "math.hpp"

  [[nodiscard]] Vec3_256 LCGRand::random_unit_vec() {
    Vec3_256 rand_vec = rand_vec_in_cube();
    normalize(rand_vec);
    return rand_vec;
  };

  [[nodiscard]] float LCGRand::rand_in_range(float min, float max) {
    float scale = lcg_rand() * rcp_rand_max;
    float f = min + scale * (max - min);
    return f;
  }

  [[nodiscard]] __m256 LCGRand::rand_in_range_256(float min, float max) {
    __m256i scale_i32 = lcg_rand_256();
    __m256 scale = _mm256_cvtepi32_ps(scale_i32);
    __m256 rcp_rand_max_vec = _mm256_broadcast_ss(&rcp_rand_max);
    scale *= rcp_rand_max_vec;

    __m256 min_vec = _mm256_broadcast_ss(&min);
    __m256 max_vec = _mm256_broadcast_ss(&max);
    __m256 range = max_vec - min_vec;

    return _mm256_fmadd_ps(scale, range, min_vec);
  }


  [[nodiscard]] Vec3_256 LCGRand::rand_vec_in_cube() {
    float min = -1.0;
    float max = 1.0;

    Vec3_256 rand_vec = {
        .x = rand_in_range_256(min, max),
        .y = rand_in_range_256(min, max),
        .z = rand_in_range_256(min, max),
    };

    return rand_vec;
  }

  [[nodiscard]] __m256i LCGRand::lcg_rand_256() {
    rseed_vec = _mm256_mullo_epi32(rseed_vec, r_a);
    rseed_vec = _mm256_add_epi32(rseed_vec, r_b);
    rseed_vec = _mm256_and_si256(rseed_vec, rand_max_vec);
    return rseed_vec;
  };

  // scalar versions of rand generation
  [[nodiscard]] int LCGRand::lcg_rand() { 
	rseed = (rseed * 1103515245u + 12345u) & RAND_MAX;
	return rseed;
}
