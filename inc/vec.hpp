#pragma once
#include <cstdint>
#include <immintrin.h>
#include "globals.hpp"

template <typename DataType>
struct _Vec3 {
  DataType x, y, z;
};

// TODO Default template typenames don't work?? Maybe they actually do... :shrug: try it later
struct Vec3_256 {
  __m256 x, y, z;
};

using Vec3 = _Vec3<float>;
using CharColor = _Vec3<uint8_t>;


namespace { // simply to remove the need for `static` on all these methods.

inline Vec3_256 operator+(const Vec3_256& a, const Vec3_256& b) {
  return Vec3_256{
      .x = _mm256_add_ps(a.x, b.x),
      .y = _mm256_add_ps(a.y, b.y),
      .z = _mm256_add_ps(a.z, b.z),
  };
}

inline Vec3_256 operator+(const Vec3_256& a, const __m256& b) {
  return Vec3_256{
      .x = _mm256_add_ps(a.x, b),
      .y = _mm256_add_ps(a.y, b),
      .z = _mm256_add_ps(a.z, b),
  };
}

inline Vec3_256& operator+=(Vec3_256& a, const Vec3_256& b) {
  a.x = _mm256_add_ps(a.x, b.x);
  a.y = _mm256_add_ps(a.y, b.y);
  a.z = _mm256_add_ps(a.z, b.z);
  return a;
}

inline Vec3_256 operator-(const Vec3_256& a, const Vec3_256& b) {
  return Vec3_256{
      .x = _mm256_sub_ps(a.x, b.x),
      .y = _mm256_sub_ps(a.y, b.y),
      .z = _mm256_sub_ps(a.z, b.z),
  };
}

inline Vec3_256& operator-=(Vec3_256& a, const Vec3_256& b) {
  a.x = _mm256_sub_ps(a.x, b.x);
  a.y = _mm256_sub_ps(a.y, b.y);
  a.z = _mm256_sub_ps(a.z, b.z);
  return a;
}

// inverse
inline Vec3_256 operator-(const Vec3_256& a) {
  // -1
  __m256 invert = _mm256_sub_ps(_mm256_setzero_ps(), global::ones);
  return Vec3_256{
      .x = _mm256_mul_ps(a.x, invert),
      .y = _mm256_mul_ps(a.y, invert),
      .z = _mm256_mul_ps(a.z, invert),
  };
}

inline Vec3_256 operator*(const Vec3_256& a, const Vec3_256& b) {
  return Vec3_256{
      .x = _mm256_mul_ps(a.x, b.x),
      .y = _mm256_mul_ps(a.y, b.y),
      .z = _mm256_mul_ps(a.z, b.z),
  };
}

inline Vec3_256 operator*(const Vec3_256& a, const __m256& b) {
  return Vec3_256{
      .x = _mm256_mul_ps(a.x, b),
      .y = _mm256_mul_ps(a.y, b),
      .z = _mm256_mul_ps(a.z, b),
  };
}

inline Vec3_256& operator*=(Vec3_256& a, const Vec3_256& b) {
  a.x = _mm256_mul_ps(a.x, b.x);
  a.y = _mm256_mul_ps(a.y, b.y);
  a.z = _mm256_mul_ps(a.z, b.z);
  return a;
}

inline Vec3_256& operator*=(Vec3_256& a, const __m256& b) {
  a.x = _mm256_mul_ps(a.x, b);
  a.y = _mm256_mul_ps(a.y, b);
  a.z = _mm256_mul_ps(a.z, b);
  return a;
}

inline Vec3_256 operator/(const Vec3_256& a, const Vec3_256& b) {

  Vec3_256 rcp_b = {
      .x = _mm256_rcp_ps(b.x),
      .y = _mm256_rcp_ps(b.y),
      .z = _mm256_rcp_ps(b.z),
  };

  return Vec3_256{
      .x = _mm256_mul_ps(a.x, rcp_b.x),
      .y = _mm256_mul_ps(a.y, rcp_b.y),
      .z = _mm256_mul_ps(a.z, rcp_b.z),
  };
}

inline Vec3_256& operator/=(Vec3_256& a, const __m256& b) {

  __m256 rcp_b = _mm256_rcp_ps(b);

  a.x = _mm256_mul_ps(a.x, rcp_b);
  a.y = _mm256_mul_ps(a.y, rcp_b);
  a.z = _mm256_mul_ps(a.z, rcp_b);
  return a;
}

inline Vec3_256 operator&(const Vec3_256& a, const __m256& b) {
  return Vec3_256{
      .x = _mm256_and_ps(a.x, b),
      .y = _mm256_and_ps(a.y, b),
      .z = _mm256_and_ps(a.z, b),
  };
}

inline Vec3_256& operator&=(Vec3_256& a, const __m256& b) {
  a.x = _mm256_and_ps(a.x, b);
  a.y = _mm256_and_ps(a.y, b);
  a.z = _mm256_and_ps(a.z, b);
  return a;
}

[[nodiscard]] inline __m256 dot(const Vec3_256* a, const Vec3_256* b) {
  __m256 dot = _mm256_mul_ps(a->x, b->x);
  dot = _mm256_fmadd_ps(a->y, b->y, dot);
  return _mm256_fmadd_ps(a->z, b->z, dot);
}

// reflect a ray about the axis
// v = v - 2*dot(v,n)*n;
[[nodiscard]] inline Vec3_256 reflect(const Vec3_256* ray_dir, const Vec3_256* axis) {
  constexpr __m256 reflect_scale = {
      2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f,
  };
  return *ray_dir - *axis * dot(ray_dir, axis) * reflect_scale;
}

[[nodiscard]] inline __m256 abs_256(__m256 vec) {
  __m256i sign_mask = _mm256_srli_epi32((__m256i)global::all_set, 1);
  return _mm256_and_ps(vec, (__m256)sign_mask);
}

[[nodiscard]] inline Vec3_256 refract(const Vec3_256* ray_dir, const Vec3_256* norm,
                                             __m256 ratio) {

  Vec3_256 inverted_ray_dir = -*ray_dir;
  __m256 cos_theta = _mm256_min_ps(dot(&inverted_ray_dir, norm), global::ones);

  Vec3_256 r_out_perp = {
      .x = _mm256_fmadd_ps(cos_theta, norm->x, ray_dir->x),
      .y = _mm256_fmadd_ps(cos_theta, norm->y, ray_dir->y),
      .z = _mm256_fmadd_ps(cos_theta, norm->z, ray_dir->z),
  };
  r_out_perp *= ratio;

  __m256 r_out_parallel_scale = global::ones - dot(&r_out_perp, &r_out_perp);

  r_out_parallel_scale = abs_256(r_out_parallel_scale);

  // square then negate
  __m256 parallel_scale_rsqrt = _mm256_rsqrt_ps(r_out_parallel_scale);
  r_out_parallel_scale *= -parallel_scale_rsqrt;

  return Vec3_256{
      .x = _mm256_fmadd_ps(r_out_parallel_scale, norm->x, r_out_perp.x),
      .y = _mm256_fmadd_ps(r_out_parallel_scale, norm->y, r_out_perp.y),
      .z = _mm256_fmadd_ps(r_out_parallel_scale, norm->z, r_out_perp.z),
  };
}

inline void normalize(Vec3_256* vec) {
  __m256 vec_len_2 = dot(vec, vec);
  __m256 recip_len = _mm256_rsqrt_ps(vec_len_2);

  *vec *= recip_len;
}

inline Vec3_256 broadcast_vec(const Vec3* vec) {
  return Vec3_256{
      .x = _mm256_broadcast_ss(&vec->x),
      .y = _mm256_broadcast_ss(&vec->y),
      .z = _mm256_broadcast_ss(&vec->z),
  };
}

inline Vec3_256 blend_vec256(const Vec3_256* a, const Vec3_256* b, __m256 mask) {
  return Vec3_256{
      .x = _mm256_blendv_ps(a->x, b->x, mask),
      .y = _mm256_blendv_ps(a->y, b->y, mask),
      .z = _mm256_blendv_ps(a->z, b->z, mask),
  };
}

inline uint32_t f_to_i(float f_val) {
  f_val += 1 << 23;
  return ((uint32_t)f_val) & 0x007FFFFF;
}

} // end of namespace (anonymous)
