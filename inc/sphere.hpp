#pragma once
#include "materials.hpp"
#include "rand.hpp"
#include "types.hpp"
#include "vec.hpp"
#include <cstdlib>
#include <cwctype>
#include <immintrin.h>
#include <limits>
#include <vector>

struct alignas(32) Sphere {
  Vec3 center;
  Material mat;
  float r;
};

struct SphereCluster {
  Vec3_256 center;
  Material_256 mat;
  __m256 r;
};

// TODO make this more dynamic like in the original rt in a weekend
static std::vector<Sphere> spheres;

namespace {
  [[gnu::always_inline]] inline void init_spheres() noexcept {
    spheres.reserve(488);
    spheres = {
        {{.center = {.x = -1.f, .y = 1.f, .z = -2.5f}, .mat = red_lambertian, .r = 1.f},
         {.center = {.x = 0.f, .y = 1.f, .z = 0.f}, .mat = glass, .r = 1.f},
         {.center = {.x = 1.f, .y = 1.f, .z = 2.5f}, .mat = copper_metallic, .r = 1.f},
         {.center = {.x = 0.f, .y = -1000.f, .z = 0.f}, .mat = silver_lambertian, .r = 1000.f}},
    };
    LCGRand lcg_rand;
    for (int a = -11; a < 11; a++) {
      for (int b = -11; b < 11; b++) {
        float choose_mat = lcg_rand.rand_in_range(0, 1);
        Vec3 center = {
            .x = static_cast<float>(a) + lcg_rand.rand_in_range(0, 1),
            .y = 0.2f,
            .z = static_cast<float>(b) + 0.9f * lcg_rand.rand_in_range(0, 1),
        };
        if (choose_mat < 0.3) {
          // diffuse
          Color albedo = {
              .x = lcg_rand.rand_in_range(0, 1),
              .y = lcg_rand.rand_in_range(0, 1),
              .z = lcg_rand.rand_in_range(0, 1),
          };
          Material new_mat = {.atten = albedo, .type = MatType::lambertian};
          spheres.push_back(Sphere{.center = center, .mat = new_mat, .r = 0.2f});
        } else if (choose_mat < 0.7) {
          // metal
          Color albedo = {
              .x = lcg_rand.rand_in_range(0.5, 1),
              .y = lcg_rand.rand_in_range(0.5, 1),
              .z = lcg_rand.rand_in_range(0.5, 1),
          };
          Material new_mat = {.atten = albedo, .type = MatType::metallic};
          spheres.push_back(Sphere{.center = center, .mat = new_mat, .r = 0.2f});
        } else {
          // glass
          Material new_mat = {.atten = white, .type = MatType::dielectric};
          spheres.push_back(Sphere{.center = center, .mat = new_mat, .r = 0.2f});
        }
      }
    }
  }

  // Returns hit t values or 0 depending on if this ray hit this sphere or not
  [[nodiscard, gnu::always_inline]]
  inline __m256 sphere_hit(const RayCluster& rays, const Sphere& sphere,
                           const float t_max) noexcept {

    Vec3_256 sphere_center = Vec3_256::broadcast_vec(sphere.center);
    Vec3_256 oc = sphere_center - rays.orig;
    float rad_2 = sphere.r * sphere.r;
    __m256 rad_2_vec = _mm256_broadcast_ss(&rad_2);

    __m256 a = rays.dir.dot(rays.dir);
    __m256 b = rays.dir.dot(oc);
    __m256 c = oc.dot(oc) - rad_2_vec;

    __m256 discrim = _mm256_fmsub_ps(b, b, a * c);

    __m256 hit_loc = _mm256_cmp_ps(discrim, global::zeros, _CMP_NLT_US);
    int no_hit = _mm256_testz_ps(hit_loc, hit_loc);

    if (no_hit) {
      return global::zeros;
    }

    // mask out the discriminants and b where there aren't hits
    discrim = _mm256_and_ps(discrim, hit_loc);
    b = _mm256_and_ps(b, hit_loc);

    __m256 sqrt_d = _mm256_sqrt_ps(discrim);
    __m256 recip_a = _mm256_rcp_ps(a);

    __m256 root = (b - sqrt_d) * recip_a;

    // allow through roots within the max t value
    __m256 t_max_vec = _mm256_broadcast_ss(&t_max);
    __m256 below_max = _mm256_cmp_ps(root, t_max_vec, _CMP_LT_OS);
    __m256 above_min = _mm256_cmp_ps(root, global::t_min_vec, _CMP_NLT_US);
    hit_loc = _mm256_and_ps(above_min, below_max);

    // Only clear materials can have another root thats worth finding.
    // This is why i only check for the farther out hit value if the material
    // is dielectric.
    if (_mm256_testz_ps(hit_loc, hit_loc) && sphere.mat.type == dielectric) {
      root = (b + sqrt_d) * recip_a;
      below_max = _mm256_cmp_ps(root, t_max_vec, _CMP_LT_OS);
      above_min = _mm256_cmp_ps(root, global::t_min_vec, _CMP_NLT_US);
      hit_loc = _mm256_and_ps(above_min, below_max);
    }
    root = _mm256_and_ps(root, hit_loc);

    return root;
  }

  [[gnu::always_inline]]
  inline void set_face_normal(const RayCluster& rays, HitRecords& hit_rec,
                              const Vec3_256& outward_norm) noexcept {
    const __m256 ray_norm_dot = rays.dir.dot(outward_norm);
    hit_rec.front_face = _mm256_cmp_ps(ray_norm_dot, _mm256_setzero_ps(), _CMP_LT_OS);
    hit_rec.norm = -outward_norm;
    hit_rec.norm = hit_rec.norm.blend_vec256(outward_norm, hit_rec.front_face);
  }

  [[gnu::always_inline]] inline void create_hit_record(HitRecords& hit_rec, const RayCluster& rays,
                                                       const SphereCluster& sphere_cluster,
                                                       const __m256& t_vals) noexcept {
    hit_rec.t = t_vals;
    hit_rec.mat = sphere_cluster.mat;

    hit_rec.orig.x = _mm256_fmadd_ps(rays.dir.x, t_vals, rays.orig.x);
    hit_rec.orig.y = _mm256_fmadd_ps(rays.dir.y, t_vals, rays.orig.y);
    hit_rec.orig.z = _mm256_fmadd_ps(rays.dir.z, t_vals, rays.orig.z);

    Vec3_256 norm = hit_rec.orig - sphere_cluster.center;
    // normalize
    norm /= sphere_cluster.r;

    set_face_normal(rays, hit_rec, norm);
  }

  // updates a sphere cluster with a sphere given a mask of where to insert the new sphere's values
  [[gnu::always_inline]] inline void update_sphere_cluster(SphereCluster& curr_cluster,
                                                           const Sphere& curr_sphere,
                                                           const __m256& update_mask) noexcept {

    if (_mm256_testz_ps(update_mask, update_mask)) {
      return;
    }

    SphereCluster new_spheres = {
        .center = Vec3_256::broadcast_vec(curr_sphere.center),
        .mat =
            {
                .atten = Vec3_256::broadcast_vec(curr_sphere.mat.atten),
                .type = _mm256_set1_epi32(curr_sphere.mat.type),
            },
        .r = _mm256_broadcast_ss(&curr_sphere.r),

    };

    new_spheres.center &= update_mask;
    new_spheres.mat.atten &= update_mask;
    new_spheres.mat.type = _mm256_and_si256(new_spheres.mat.type, (__m256i)update_mask);
    new_spheres.r = _mm256_and_ps(new_spheres.r, update_mask);

    // negation of update locations so we can preserve current values
    // while clearing bits where we will update
    __m256 preserve_curr = _mm256_xor_ps(update_mask, (__m256)global::all_set);

    SphereCluster curr_spheres = {
        .center = curr_cluster.center & preserve_curr,
        .mat =
            {
                .atten = curr_cluster.mat.atten & preserve_curr,
                .type = _mm256_and_si256(curr_cluster.mat.type, (__m256i)preserve_curr),
            },
        .r = _mm256_and_ps(curr_cluster.r, preserve_curr),

    };

    curr_cluster.center = new_spheres.center + curr_spheres.center;
    curr_cluster.mat.atten = new_spheres.mat.atten + curr_spheres.mat.atten;
    curr_cluster.mat.type = new_spheres.mat.type + curr_spheres.mat.type;
    curr_cluster.r = new_spheres.r + curr_spheres.r;
  };

  [[gnu::always_inline]] inline void find_sphere_hits(HitRecords& hit_rec, const RayCluster& rays,
                                                      const float t_max) noexcept {

    SphereCluster closest_spheres = {
        .center =
            {
                .x = global::zeros,
                .y = global::zeros,
                .z = global::zeros,
            },
        .mat = Material_256{},
        .r = global::zeros,
    };

    constexpr auto flt_max = std::numeric_limits<float>::max();
    __m256 max = _mm256_broadcast_ss(&flt_max);

    // find first sphere as a baseline
    __m256 lowest_t_vals = sphere_hit(rays, spheres[0], t_max);
    __m256 hit_loc = _mm256_cmp_ps(lowest_t_vals, global::zeros, _CMP_NEQ_UQ);

    update_sphere_cluster(closest_spheres, spheres[0], hit_loc);

    for (size_t i = 1; i < spheres.size(); i++) {
      __m256 new_t_vals = sphere_hit(rays, spheres[i], t_max);

      // don't update on instances of no hits (hit locations all zeros)
      hit_loc = _mm256_cmp_ps(new_t_vals, global::zeros, _CMP_NEQ_UQ);
      if (_mm256_testz_ps(hit_loc, hit_loc)) {
        continue;
      }

      // replace all 0's with float maximum to not replace actual values with
      // 0's during the minimum comparisons. Again, 0's represent no hits
      __m256 no_hit_loc = _mm256_xor_ps(hit_loc, (__m256)global::all_set);
      __m256 max_mask = _mm256_and_ps(no_hit_loc, max);
      new_t_vals = _mm256_or_ps(new_t_vals, max_mask);

      // replace 0's with max for current lowest too
      __m256 curr_no_hit_loc = _mm256_cmp_ps(lowest_t_vals, global::zeros, _CMP_EQ_OQ);
      max_mask = _mm256_and_ps(curr_no_hit_loc, max);
      __m256 lowest_t_masked = _mm256_or_ps(lowest_t_vals, max_mask);

      // update sphere references based on where new
      // t values are closer than the current lowest
      __m256 update_locs = _mm256_cmp_ps(new_t_vals, lowest_t_masked, _CMP_LT_OS);
      update_sphere_cluster(closest_spheres, spheres[i], update_locs);

      // update current lowest t values based on new t's, however, mask out
      // where we put float max values so that the t values still represent
      // no hits as 0.0
      lowest_t_vals = _mm256_min_ps(lowest_t_masked, new_t_vals);
      __m256 actual_vals_loc = _mm256_cmp_ps(lowest_t_vals, max, _CMP_NEQ_UQ);
      lowest_t_vals = _mm256_and_ps(lowest_t_vals, actual_vals_loc);
    }

    create_hit_record(hit_rec, rays, closest_spheres, lowest_t_vals);
  }

} // namespace
