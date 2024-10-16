#pragma once
#include "colors.hpp"
#include "globals.hpp"
#include "rand.hpp"
#include "types.hpp"
#include <cmath>
#include <cstdlib>
#include <immintrin.h>

enum MatType {
  metallic,
  lambertian,
  dielectric,
};

struct alignas(16) Material {
  Color atten;
  MatType type;
};

namespace {

  using namespace colors;

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

  alignas(32) constexpr int metallic_types[8] = {
      MatType::metallic, MatType::metallic, MatType::metallic, MatType::metallic,
      MatType::metallic, MatType::metallic, MatType::metallic, MatType::metallic,
  };

  alignas(32) constexpr int lambertian_types[8] = {
      MatType::lambertian, MatType::lambertian, MatType::lambertian, MatType::lambertian,
      MatType::lambertian, MatType::lambertian, MatType::lambertian, MatType::lambertian,
  };

  alignas(32) constexpr int dielectric_types[8] = {
      MatType::dielectric, MatType::dielectric, MatType::dielectric, MatType::dielectric,
      MatType::dielectric, MatType::dielectric, MatType::dielectric, MatType::dielectric,
  };

  LCGRand lcg_rand; // TODO move this someplace else? Why is it here? Is it thread_local?

  [[gnu::always_inline]] inline void scatter_metallic(RayCluster& rays, const HitRecords& hit_rec) {
    Vec3_256 reflected = rays.dir.reflect(hit_rec.norm);
    reflected.normalize();

    __m256 dp = reflected.dot(hit_rec.norm);
    __m256 greater_than_zero = _mm256_cmp_ps(dp, global::zeros, _CMP_NLE_US);
    rays.dir = reflected & greater_than_zero;
  }

  [[nodiscard, gnu::always_inline]] inline __m256 near_zero(const Vec3_256* vec) {
    __m256 near_x = _mm256_cmp_ps(abs_256(vec->x), global::t_min_vec, _CMP_LT_OS);
    __m256 near_y = _mm256_cmp_ps(abs_256(vec->y), global::t_min_vec, _CMP_LT_OS);
    __m256 near_z = _mm256_cmp_ps(abs_256(vec->z), global::t_min_vec, _CMP_LT_OS);

    return _mm256_and_ps(near_x, _mm256_and_ps(near_y, near_z));
  }

  [[gnu::always_inline]] inline void scatter_lambertian(RayCluster& rays,
                                                        const HitRecords& hit_rec) {
    Vec3_256 rand_vec = lcg_rand.random_unit_vec();
    Vec3_256 scatter_dir = rand_vec + hit_rec.norm;

    //  rays->dir = blend_vec256(&scatter_dir, &hit_rec->norm, near_zero(&scatter_dir));
    rays.dir = scatter_dir;
  }

  [[nodiscard, gnu::always_inline]] inline __m256 reflectance(__m256 cos, __m256 ref_idx) {
    __m256 ref_low = global::ones - ref_idx;
    __m256 ref_high = global::ones + ref_idx;
    ref_high = _mm256_rcp_ps(ref_high);
    __m256 ref = ref_low * ref_high;
    ref *= ref;

    __m256 cos_sub = global::ones - cos;
    // cos_sub^5
    __m256 cos_5 = cos_sub * cos_sub;
    cos_5 *= cos_sub;
    cos_5 *= cos_sub;
    cos_5 *= cos_sub;

    __m256 ref_sub = global::ones - ref;
    return _mm256_fmadd_ps(ref_sub, cos_5, ref);
  }

  [[gnu::always_inline]] inline void scatter_dielectric(RayCluster& rays,
                                                        const HitRecords& hit_rec) {

    __m256 ri = _mm256_blendv_ps(global::ir_vec, global::rcp_ir_vec, hit_rec.front_face);
    Vec3_256 unit_dir = rays.dir;
    unit_dir.normalize();

    Vec3_256 inverse_unit_dir = -unit_dir;

    __m256 cos_theta = inverse_unit_dir.dot(hit_rec.norm);
    cos_theta = _mm256_min_ps(cos_theta, global::ones);

    __m256 sin_theta = _mm256_sqrt_ps(global::ones - cos_theta * cos_theta);

    __m256 can_refract = ri * sin_theta;
    can_refract = _mm256_cmp_ps(can_refract, global::ones, _CMP_LE_OS);

    __m256 ref = reflectance(cos_theta, ri);
    __m256 rand_vec = lcg_rand.rand_in_range_256(0.f, 1.f);
    __m256 low_reflectance_loc = _mm256_cmp_ps(ref, rand_vec, _CMP_LE_OS);
    __m256 refraction_loc = _mm256_and_ps(can_refract, low_reflectance_loc);
    __m256 reflection_loc = _mm256_xor_ps(refraction_loc, (__m256)global::all_set);

    if (!_mm256_testz_ps(refraction_loc, refraction_loc)) {
      Vec3_256 refract_dir = unit_dir.refract(hit_rec.norm, ri);
      rays.dir = rays.dir.blend_vec256(refract_dir, refraction_loc);
    }
    if (!_mm256_testz_ps(reflection_loc, reflection_loc)) {
      Vec3_256 reflect_dir = unit_dir.reflect(hit_rec.norm);

      reflection_loc = _mm256_and_ps(reflection_loc, hit_rec.front_face);
      rays.dir = rays.dir.blend_vec256(reflect_dir, reflection_loc);
    }
  }

  [[gnu::always_inline]] inline void scatter(RayCluster& rays, const HitRecords& hit_rec) {
    __m256i metallic_type = _mm256_load_si256((__m256i*)metallic_types);
    __m256i lambertian_type = _mm256_load_si256((__m256i*)lambertian_types);
    __m256i dielectric_type = _mm256_load_si256((__m256i*)dielectric_types);

    __m256i metallic_loc = _mm256_cmpeq_epi32(hit_rec.mat.type, metallic_type);
    __m256i lambertian_loc = _mm256_cmpeq_epi32(hit_rec.mat.type, lambertian_type);
    __m256i dielectric_loc = _mm256_cmpeq_epi32(hit_rec.mat.type, dielectric_type);

    if (!_mm256_testz_si256(metallic_loc, metallic_loc)) {
      RayCluster metallic_rays = {
          .dir = rays.dir,
          .orig = hit_rec.orig,
      };
      scatter_metallic(metallic_rays, hit_rec);

      rays.dir = rays.dir.blend_vec256(metallic_rays.dir, (__m256)metallic_loc);
      rays.orig = rays.orig.blend_vec256(metallic_rays.orig, (__m256)metallic_loc);
    }
    if (!_mm256_testz_si256(lambertian_loc, lambertian_loc)) {
      RayCluster lambertian_rays = {
          .dir = rays.dir,
          .orig = hit_rec.orig,
      };
      scatter_lambertian(lambertian_rays, hit_rec);

      rays.dir = rays.dir.blend_vec256(lambertian_rays.dir, (__m256)lambertian_loc);
      rays.orig = rays.orig.blend_vec256(lambertian_rays.orig, (__m256)lambertian_loc);
    }
    if (!_mm256_testz_si256(dielectric_loc, dielectric_loc)) {
      RayCluster dielectric_rays = {
          .dir = rays.dir,
          .orig = hit_rec.orig,
      };
      scatter_dielectric(dielectric_rays, hit_rec);

      rays.dir = rays.dir.blend_vec256(dielectric_rays.dir, (__m256)dielectric_loc);
      rays.orig = rays.orig.blend_vec256(dielectric_rays.orig, (__m256)dielectric_loc);
    }
  }

} // namespace
