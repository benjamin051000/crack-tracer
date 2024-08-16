#pragma once
#include "globals.h"
#include "math.h"
#include "rand.h"
#include "types.h"
#include <cstdlib>
#include <immintrin.h>

constexpr Color silver = {.x = 0.5f, .y = 0.5f, .z = 0.5f};
constexpr Color grey = {.x = 0.5f, .y = 0.5f, .z = 0.5f};
constexpr Color white = {.x = 1.f, .y = 1.f, .z = 1.f};
constexpr Color red = {.x = 0.90f, .y = 0.20f, .z = 0.20f};
constexpr Color gold = {.x = 0.90f, .y = 0.75f, .z = 0.54f};
constexpr Color copper = {.x = 0.59f, .y = 0.34f, .z = 0.29f};
constexpr Color green = {.x = 0.f, .y = 1.f, .z = 0.f};
constexpr Color moon = {.x = 100.f, .y = 100.f, .z = 100.f};

constexpr Material silver_metallic = {.atten = silver, .type = MatType::metallic};
constexpr Material red_metallic = {.atten = red, .type = MatType::metallic};
constexpr Material gold_metallic = {.atten = gold, .type = MatType::metallic};
constexpr Material copper_metallic = {.atten = copper, .type = MatType::metallic};
constexpr Material green_metallic = {.atten = green, .type = MatType::metallic};

constexpr Material silver_lambertian = {.atten = silver, .type = MatType::lambertian};
constexpr Material red_lambertian = {.atten = red, .type = MatType::lambertian};
constexpr Material gold_lambertian = {.atten = gold, .type = MatType::lambertian};
constexpr Material star_lambertian = {.atten = moon, .type = MatType::lambertian};
constexpr Material grey_lambertian = {.atten = grey, .type = MatType::lambertian};

constexpr Material glass = {.atten = white, .type = MatType::dielectric};

alignas(32) static const int metallic_types[8] = {
    MatType::metallic, MatType::metallic, MatType::metallic, MatType::metallic,
    MatType::metallic, MatType::metallic, MatType::metallic, MatType::metallic,
};

alignas(32) static const int lambertian_types[8] = {
    MatType::lambertian, MatType::lambertian, MatType::lambertian, MatType::lambertian,
    MatType::lambertian, MatType::lambertian, MatType::lambertian, MatType::lambertian,
};

alignas(32) static const int dielectric_types[8] = {
    MatType::dielectric, MatType::dielectric, MatType::dielectric, MatType::dielectric,
    MatType::dielectric, MatType::dielectric, MatType::dielectric, MatType::dielectric,
};

static LCGRand lcg_rand;
inline static void scatter_metallic(RayCluster* rays, const HitRecords* hit_rec) {
  reflect(&rays->dir, &hit_rec->norm);
};

inline static void scatter_lambertian(RayCluster* rays, const HitRecords* hit_rec) {
  Vec3_256 rand_vec = lcg_rand.random_unit_vec();
  rays->dir = rand_vec + hit_rec->norm;
}

[[nodiscard]] inline static __m256 reflectance(__m256 cos, __m256 ref_idx) {
  __m256 ref_low = global::white - ref_idx;
  __m256 ref_high = global::white + ref_idx;
  ref_high = _mm256_rcp_ps(ref_high);
  __m256 ref = ref_low * ref_high;
  ref *= ref;

  __m256 cos_sub = global::white - cos;
  // cos_sub^5
  __m256 cos_5 = cos_sub * cos_sub;
  cos_5 *= cos_sub;
  cos_5 *= cos_sub;
  cos_5 *= cos_sub;

  __m256 ref_sub = global::white - ref;
  return _mm256_fmadd_ps(ref_sub, cos_5, ref);
}

inline static void scatter_dielectric(RayCluster* rays, const HitRecords* hit_rec) {
  __m256 refraction_ratio =
      _mm256_blendv_ps(global::ir_vec, global::rcp_ir_vec, hit_rec->front_face);

  Vec3_256 unit_dir = rays->dir;
  normalize(&unit_dir);
  unit_dir = -unit_dir;

  __m256 cos_theta = dot(&unit_dir, &hit_rec->norm);
  cos_theta = _mm256_min_ps(cos_theta, global::white);

  __m256 sin_theta = cos_theta * cos_theta;
  sin_theta = global::white - sin_theta;
  sin_theta = _mm256_sqrt_ps(sin_theta);

  __m256 can_refract = refraction_ratio * sin_theta;
  can_refract = _mm256_cmp_ps(can_refract, global::white, global::cmple);

  __m256 ref = reflectance(cos_theta, refraction_ratio);
  __m256 rand_vec = lcg_rand.rand_in_range_256(0.2f, 1.f);
  __m256 low_reflectance_loc = _mm256_cmp_ps(ref, rand_vec, global::cmple);
  __m256 refraction_loc = _mm256_and_ps(can_refract, low_reflectance_loc);
  __m256 reflection_loc = _mm256_xor_ps(refraction_loc, global::all_set);

  if (!_mm256_testz_ps(refraction_loc, refraction_loc)) {
    Vec3_256 refract_dir = rays->dir;
    refract(&refract_dir, &hit_rec->norm, refraction_ratio);

    rays->dir = blend_vec256(&rays->dir, &refract_dir, refraction_loc);
  }
  if (!_mm256_testz_ps(reflection_loc, reflection_loc)) {
    Vec3_256 reflect_dir = rays->dir;
    reflect(&reflect_dir, &hit_rec->norm);

    rays->dir = blend_vec256(&rays->dir, &reflect_dir, reflection_loc);
  }
}

inline static void scatter(RayCluster* rays, const HitRecords* hit_rec) {
  __m256i metallic_type = _mm256_load_si256((__m256i*)metallic_types);
  __m256i lambertian_type = _mm256_load_si256((__m256i*)lambertian_types);
  __m256i dielectric_type = _mm256_load_si256((__m256i*)dielectric_types);

  __m256i metallic_loc = _mm256_cmpeq_epi32(hit_rec->mat.type, metallic_type);
  __m256i lambertian_loc = _mm256_cmpeq_epi32(hit_rec->mat.type, lambertian_type);
  __m256i dielectric_loc = _mm256_cmpeq_epi32(hit_rec->mat.type, dielectric_type);

  if (!_mm256_testz_si256(metallic_loc, metallic_loc)) {
    RayCluster metallic_rays = {
        .dir = rays->dir,
        .orig = hit_rec->orig,
    };
    scatter_metallic(&metallic_rays, hit_rec);

    rays->dir = blend_vec256(&rays->dir, &metallic_rays.dir, (__m256)metallic_loc);
    rays->orig = blend_vec256(&rays->orig, &metallic_rays.orig, (__m256)metallic_loc);
  }
  if (!_mm256_testz_si256(lambertian_loc, lambertian_loc)) {
    RayCluster lambertian_rays = {
        .dir = rays->dir,
        .orig = hit_rec->orig,
    };
    scatter_lambertian(&lambertian_rays, hit_rec);

    rays->dir = blend_vec256(&rays->dir, &lambertian_rays.dir, (__m256)lambertian_loc);
    rays->orig = blend_vec256(&rays->orig, &lambertian_rays.orig, (__m256)lambertian_loc);
  }
  if (!_mm256_testz_si256(dielectric_loc, dielectric_loc)) {
    RayCluster dielectric_rays = {
        .dir = rays->dir,
        .orig = hit_rec->orig,
    };
    scatter_dielectric(&dielectric_rays, hit_rec);

    rays->dir = blend_vec256(&rays->dir, &dielectric_rays.dir, (__m256)dielectric_loc);
    rays->orig = blend_vec256(&rays->orig, &dielectric_rays.orig, (__m256)dielectric_loc);
  }
}
