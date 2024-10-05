#pragma once
#include "materials.hpp"
#include "rand.hpp"
#include "types.hpp"
#include <cstdlib>
#include <immintrin.h>
#include <vector>

static std::vector<Sphere> spheres;
inline static void init_spheres() {
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
          .x = a + lcg_rand.rand_in_range(0, 1),
          .y = 0.2f,
          .z = b + 0.9f * lcg_rand.rand_in_range(0, 1),
      };
      if (choose_mat < 0.3) {
        // diffuse
        Color albedo = {
            .x = lcg_rand.rand_in_range(0, 1),
            .y = lcg_rand.rand_in_range(0, 1),
            .z = lcg_rand.rand_in_range(0, 1),
        };
        Material new_mat = {.atten = albedo, .type = MatType::lambertian};
        spheres.push_back(Sphere{.center = center, .mat = new_mat, .r = 0.2});
      } else if (choose_mat < 0.7) {
        // metal
        Color albedo = {
            .x = lcg_rand.rand_in_range(0.5, 1),
            .y = lcg_rand.rand_in_range(0.5, 1),
            .z = lcg_rand.rand_in_range(0.5, 1),
        };
        Material new_mat = {.atten = albedo, .type = MatType::metallic};
        spheres.push_back(Sphere{.center = center, .mat = new_mat, .r = 0.2});
      } else {
        // glass
        Material new_mat = {.atten = white, .type = MatType::dielectric};
        spheres.push_back(Sphere{.center = center, .mat = new_mat, .r = 0.2});
      }
    }
  }
}

// Returns hit t values or 0 depending on if this ray hit this sphere or not
[[nodiscard]] inline static __m256 sphere_hit(const RayCluster* rays, const Sphere* sphere,
                                              float t_max) {

  Vec3_256 sphere_center = broadcast_vec(&sphere->center);
  Vec3_256 oc = sphere_center - rays->orig;
  float rad_2 = sphere->r * sphere->r;
  __m256 rad_2_vec = _mm256_broadcast_ss(&rad_2);

  __m256 a = dot(&rays->dir, &rays->dir);
  __m256 b = dot(&rays->dir, &oc);
  __m256 c = dot(&oc, &oc) - rad_2_vec;

  __m256 discrim = _mm256_fmsub_ps(b, b, a * c);

  __m256 hit_loc = _mm256_cmp_ps(discrim, global::zeros, global::cmpnlt);
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
  __m256 below_max = _mm256_cmp_ps(root, t_max_vec, global::cmplt);
  __m256 above_min = _mm256_cmp_ps(root, global::t_min_vec, global::cmpnlt);
  hit_loc = _mm256_and_ps(above_min, below_max);

  // Only clear materials can have another root thats worth finding.
  // This is why i only check for the farther out hit value if the material
  // is dielectric.
  if (_mm256_testz_ps(hit_loc, hit_loc) && sphere->mat.type == dielectric) {
    root = (b + sqrt_d) * recip_a;
    below_max = _mm256_cmp_ps(root, t_max_vec, global::cmplt);
    above_min = _mm256_cmp_ps(root, global::t_min_vec, global::cmpnlt);
    hit_loc = _mm256_and_ps(above_min, below_max);
  }
  root = _mm256_and_ps(root, hit_loc);

  return root;
}

inline static void set_face_normal(const RayCluster* rays, HitRecords* hit_rec,
                                   const Vec3_256* outward_norm) {
  __m256 ray_norm_dot = dot(&rays->dir, outward_norm);
  hit_rec->front_face = _mm256_cmp_ps(ray_norm_dot, _mm256_setzero_ps(), global::cmplt);
  hit_rec->norm = -*outward_norm;
  hit_rec->norm = blend_vec256(&hit_rec->norm, outward_norm, hit_rec->front_face);
}

inline static void create_hit_record(HitRecords* hit_rec, const RayCluster* rays,
                                     SphereCluster* sphere_cluster, __m256 t_vals) {
  hit_rec->t = t_vals;
  hit_rec->mat = sphere_cluster->mat;

  hit_rec->orig.x = _mm256_fmadd_ps(rays->dir.x, t_vals, rays->orig.x);
  hit_rec->orig.y = _mm256_fmadd_ps(rays->dir.y, t_vals, rays->orig.y);
  hit_rec->orig.z = _mm256_fmadd_ps(rays->dir.z, t_vals, rays->orig.z);

  Vec3_256 norm = hit_rec->orig - sphere_cluster->center;
  // normalize
  norm /= sphere_cluster->r;

  set_face_normal(rays, hit_rec, &norm);
}

// updates a sphere cluster with a sphere given a mask of where to insert the new sphere's values
inline static void update_sphere_cluster(SphereCluster* curr_cluster, Sphere curr_sphere,
                                         __m256 update_mask) {

  if (_mm256_testz_ps(update_mask, update_mask)) {
    return;
  }

  SphereCluster new_spheres = {
      .center = broadcast_vec(&curr_sphere.center),
      .mat =
          {
              .atten = broadcast_vec(&curr_sphere.mat.atten),
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
      .center = curr_cluster->center & preserve_curr,
      .mat =
          {
              .atten = curr_cluster->mat.atten & preserve_curr,
              .type = _mm256_and_si256(curr_cluster->mat.type, (__m256i)preserve_curr),
          },
      .r = _mm256_and_ps(curr_cluster->r, preserve_curr),

  };

  curr_cluster->center = new_spheres.center + curr_spheres.center;
  curr_cluster->mat.atten = new_spheres.mat.atten + curr_spheres.mat.atten;
  curr_cluster->mat.type = new_spheres.mat.type + curr_spheres.mat.type;
  curr_cluster->r = new_spheres.r + curr_spheres.r;
};

inline static void find_sphere_hits(HitRecords* hit_rec, const RayCluster* rays, float t_max) {

  SphereCluster closest_spheres = {
      .center =
          {
              .x = global::zeros,
              .y = global::zeros,
              .z = global::zeros,
          },
      .r = global::zeros,
  };

  __m256 max = _mm256_broadcast_ss(&global::float_max);

  // find first sphere as a baseline
  __m256 lowest_t_vals = sphere_hit(rays, &spheres[0], t_max);
  __m256 hit_loc = _mm256_cmp_ps(lowest_t_vals, global::zeros, global::cmpneq);

  update_sphere_cluster(&closest_spheres, spheres[0], hit_loc);

  for (size_t i = 1; i < spheres.size(); i++) {
    __m256 new_t_vals = sphere_hit(rays, &spheres[i], t_max);

    // don't update on instances of no hits (hit locations all zeros)
    hit_loc = _mm256_cmp_ps(new_t_vals, global::zeros, global::cmpneq);
    if (_mm256_testz_ps(hit_loc, hit_loc)) {
      continue;
    }

    // replace all 0's with float maximum to not replace actual values with
    // 0's during the minimum comparisons. Again, 0's represent no hits
    __m256 no_hit_loc = _mm256_xor_ps(hit_loc, (__m256)global::all_set);
    __m256 max_mask = _mm256_and_ps(no_hit_loc, max);
    new_t_vals = _mm256_or_ps(new_t_vals, max_mask);

    // replace 0's with max for current lowest too
    __m256 curr_no_hit_loc = _mm256_cmp_ps(lowest_t_vals, global::zeros, global::cmpeq);
    max_mask = _mm256_and_ps(curr_no_hit_loc, max);
    __m256 lowest_t_masked = _mm256_or_ps(lowest_t_vals, max_mask);

    // update sphere references based on where new
    // t values are closer than the current lowest
    __m256 update_locs = _mm256_cmp_ps(new_t_vals, lowest_t_masked, global::cmplt);
    update_sphere_cluster(&closest_spheres, spheres[i], update_locs);

    // update current lowest t values based on new t's, however, mask out
    // where we put float max values so that the t values still represent
    // no hits as 0.0
    lowest_t_vals = _mm256_min_ps(lowest_t_masked, new_t_vals);
    __m256 actual_vals_loc = _mm256_cmp_ps(lowest_t_vals, max, global::cmpneq);
    lowest_t_vals = _mm256_and_ps(lowest_t_vals, actual_vals_loc);
  }

  create_hit_record(hit_rec, rays, &closest_spheres, lowest_t_vals);
}
