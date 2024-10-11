#pragma once
#include <cstdint>
#include <immintrin.h>
#include "globals.hpp"

namespace { // simply to remove the need for `static` on all these methods.

[[nodiscard, gnu::always_inline]] inline __m256 abs_256(const __m256& vec) noexcept {
  const __m256i sign_mask = _mm256_srli_epi32((__m256i)global::all_set, 1);
  return _mm256_and_ps(vec, (__m256)sign_mask);
}

} // end of namespace (anonymous)
template <typename DataType>
struct _Vec3 {
  DataType x, y, z;
};

using Vec3 = _Vec3<float>;
using CharColor = _Vec3<uint8_t>;

// TODO Default template typenames don't work?? Maybe they actually do... :shrug: try it later
struct Vec3_256 {
  __m256 x, y, z;

[[nodiscard, gnu::always_inline]] inline Vec3_256 operator+(const Vec3_256& b) const noexcept {
  return Vec3_256 {
      _mm256_add_ps(x, b.x),
      _mm256_add_ps(y, b.y),
      _mm256_add_ps(z, b.z),
  };
}

[[nodiscard, gnu::always_inline]] inline Vec3_256 operator+(const __m256& b) const noexcept {
  return Vec3_256{
      _mm256_add_ps(x, b),
      _mm256_add_ps(y, b),
      _mm256_add_ps(z, b),
  };
}

[[gnu::always_inline]] inline Vec3_256& operator+=(const Vec3_256& b) noexcept {
  x = _mm256_add_ps(x, b.x);
  y = _mm256_add_ps(y, b.y);
  z = _mm256_add_ps(z, b.z);
  return *this;
}

[[nodiscard, gnu::always_inline]] inline Vec3_256 operator-(const Vec3_256& b) const noexcept {
  return Vec3_256{
      _mm256_sub_ps(x, b.x),
      _mm256_sub_ps(y, b.y),
      _mm256_sub_ps(z, b.z),
  };
}

[[nodiscard, gnu::always_inline]] inline Vec3_256& operator-=(const Vec3_256& b) noexcept {
  x = _mm256_sub_ps(x, b.x);
  y = _mm256_sub_ps(y, b.y);
  z = _mm256_sub_ps(z, b.z);
  return *this;
}

	/** Inverse
	* Multiplies by -1
	*/
[[nodiscard, gnu::always_inline]] inline Vec3_256 operator-() const noexcept {
  const __m256 negative_one = _mm256_sub_ps(_mm256_setzero_ps(), global::ones);
  return Vec3_256 {
      _mm256_mul_ps(x, negative_one),
      _mm256_mul_ps(y, negative_one),
      _mm256_mul_ps(z, negative_one),
  };
}

[[nodiscard, gnu::always_inline]] inline Vec3_256 operator*(const Vec3_256& b) const noexcept {
  return Vec3_256 {
      _mm256_mul_ps(x, b.x),
      _mm256_mul_ps(y, b.y),
      _mm256_mul_ps(z, b.z),
  };
}

[[nodiscard, gnu::always_inline]] inline Vec3_256 operator*(const __m256& b) const noexcept {
  return Vec3_256 {
      _mm256_mul_ps(x, b),
      _mm256_mul_ps(y, b),
      _mm256_mul_ps(z, b),
  };
}

[[gnu::always_inline]] inline Vec3_256& operator*=(const Vec3_256& b) noexcept {
  x = _mm256_mul_ps(x, b.x);
  y = _mm256_mul_ps(y, b.y);
  z = _mm256_mul_ps(z, b.z);
  return *this;
}

[[gnu::always_inline]] inline Vec3_256& operator*=(const __m256& b) noexcept {
  x = _mm256_mul_ps(x, b);
  y = _mm256_mul_ps(y, b);
  z = _mm256_mul_ps(z, b);
  return *this;
}

[[nodiscard, gnu::always_inline]] inline Vec3_256 operator/(const Vec3_256& b) const noexcept {
  const Vec3_256 rcp_b {
      _mm256_rcp_ps(b.x),
      _mm256_rcp_ps(b.y),
      _mm256_rcp_ps(b.z),
  };

  return Vec3_256 {
      _mm256_mul_ps(x, rcp_b.x),
      _mm256_mul_ps(y, rcp_b.y),
      _mm256_mul_ps(z, rcp_b.z),
  };
}

[[gnu::always_inline]] inline Vec3_256& operator/=(const __m256& b) noexcept {
  const __m256 rcp_b = _mm256_rcp_ps(b);
  x = _mm256_mul_ps(x, rcp_b);
  y = _mm256_mul_ps(y, rcp_b);
  z = _mm256_mul_ps(z, rcp_b);
  return *this;
}

[[nodiscard, gnu::always_inline]] inline Vec3_256 operator&(const __m256& b) const noexcept {
  return Vec3_256 {
      _mm256_and_ps(x, b),
      _mm256_and_ps(y, b),
      _mm256_and_ps(z, b),
  };
}

[[gnu::always_inline]] inline Vec3_256& operator&=(const __m256& b) noexcept {
  x = _mm256_and_ps(x, b);
  y = _mm256_and_ps(y, b);
  z = _mm256_and_ps(z, b);
  return *this;
}

[[nodiscard, gnu::always_inline]] inline __m256 dot(const Vec3_256& b) const noexcept {
  __m256 dot = _mm256_mul_ps(x, b.x);
  dot = _mm256_fmadd_ps(y, b.y, dot);
  return _mm256_fmadd_ps(z, b.z, dot);
}

// reflect a ray about the axis
// v = v - 2*dot(v,n)*n;
[[nodiscard, gnu::always_inline]] inline Vec3_256 reflect(const Vec3_256& axis) const noexcept {
  constexpr __m256 reflect_scale = {2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f, 2.f};
  return *this - axis * dot(axis) * reflect_scale;
}


[[nodiscard, gnu::always_inline]] inline Vec3_256 refract(
	const Vec3_256& norm,
	const __m256& ratio
) const noexcept {
  const Vec3_256 inverted_ray_dir = -(*this);
  const __m256 cos_theta = _mm256_min_ps(inverted_ray_dir.dot(norm), global::ones);

  Vec3_256 r_out_perp {
      _mm256_fmadd_ps(cos_theta, norm.x, x),
      _mm256_fmadd_ps(cos_theta, norm.y, y),
      _mm256_fmadd_ps(cos_theta, norm.z, z),
  };
  r_out_perp *= ratio;

  __m256 r_out_parallel_scale = global::ones - r_out_perp.dot(r_out_perp);
  r_out_parallel_scale = abs_256(r_out_parallel_scale);

  // square then negate
  const __m256 parallel_scale_rsqrt = _mm256_rsqrt_ps(r_out_parallel_scale);
  r_out_parallel_scale *= -parallel_scale_rsqrt;

  return Vec3_256 {
      _mm256_fmadd_ps(r_out_parallel_scale, norm.x, r_out_perp.x),
      _mm256_fmadd_ps(r_out_parallel_scale, norm.y, r_out_perp.y),
      _mm256_fmadd_ps(r_out_parallel_scale, norm.z, r_out_perp.z),
  };
}

[[gnu::always_inline]] inline void normalize() noexcept {
  const __m256 vec_len_2 = dot(*this);
  const __m256 recip_len = _mm256_rsqrt_ps(vec_len_2);
  *this *= recip_len;
}

[[nodiscard, gnu::always_inline]] static inline Vec3_256 broadcast_vec(const Vec3& vec) noexcept {
  return Vec3_256 {
      _mm256_broadcast_ss(&vec.x),
      _mm256_broadcast_ss(&vec.y),
      _mm256_broadcast_ss(&vec.z),
  };
}

[[nodiscard, gnu::always_inline]] inline Vec3_256 blend_vec256(
	const Vec3_256& b,
	const __m256& mask
) const noexcept {
  return Vec3_256 {
      _mm256_blendv_ps(x, b.x, mask),
      _mm256_blendv_ps(y, b.y, mask),
      _mm256_blendv_ps(z, b.z, mask),
  };
}

}; // end of struct Vec3_256
