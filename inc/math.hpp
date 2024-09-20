#pragma once
#include "types.hpp"
#include <cstdint>
#include <cstdio>
#include <immintrin.h>

[[nodiscard]] Vec3_256 operator+(const Vec3_256& a, const Vec3_256& b);

[[nodiscard]] Vec3_256 operator+(const Vec3_256& a, const __m256& b);

 Vec3_256& operator+=(Vec3_256& a, const Vec3_256& b);

[[nodiscard]] Vec3_256 operator-(const Vec3_256& a, const Vec3_256& b);

 Vec3_256& operator-=(Vec3_256& a, const Vec3_256& b);

// inverse
[[nodiscard]] Vec3_256 operator-(const Vec3_256& a);

[[nodiscard]] Vec3_256 operator*(const Vec3_256& a, const Vec3_256& b);

[[nodiscard]] Vec3_256 operator*(const Vec3_256& a, const __m256& b);

 Vec3_256& operator*=(Vec3_256& a, const Vec3_256& b);

 Vec3_256& operator*=(Vec3_256& a, const __m256& b);

[[nodiscard]] Vec3_256 operator/(const Vec3_256& a, const Vec3_256& b);

 Vec3_256& operator/=(Vec3_256& a, const __m256& b);

 Vec3_256 operator&(const Vec3_256& a, const __m256& b);

 Vec3_256& operator&=(Vec3_256& a, const __m256& b);

[[nodiscard]] __m256 dot(const Vec3_256& a, const Vec3_256& b);

// reflect a ray about the axis
// v = v - 2*dot(v,n)*n;
[[nodiscard]] Vec3_256 reflect(const Vec3_256& ray_dir, const Vec3_256& axis);

[[nodiscard]] __m256 abs_256(const __m256 vec);

[[nodiscard]] Vec3_256 refract(const Vec3_256& ray_dir, const Vec3_256& norm, const __m256 ratio);

 void normalize(Vec3_256& vec);

[[nodiscard]] Vec3_256 broadcast_vec(const Vec3& vec);

[[nodiscard]] Vec3_256 blend_vec256(const Vec3_256& a, const Vec3_256& b, const __m256 mask);

[[nodiscard]] uint32_t f_to_i(float f_val);
