#include "../src/math.h"
#include "../src/render.h"
#include "../src/sphere.h"
#include "../src/utils.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <immintrin.h>

const RayCluster rays{.dir = {
                          .x = _mm256_set_ps(-.6f, -.2f, .2f, .6f, -.6f, -.2f, .2f, .6f),
                          .y = _mm256_set_ps(.4f, .4f, .4f, .4f, -.4f, -.4f, -.4f, -.4f),
                          .z = _mm256_set_ps(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f),
                      }};

const Sphere sphere{
    .center = {.x = 0.f, .y = 0.f, .z = -1.f},
    .mat = silver_metallic,
    .r = .5f,
};

void test_sphere_hit() {
  printf("TESTING SPHERE_HIT\n");
  __m256 t_vals = sphere_hit(&rays, &sphere, 1000.f);
  print_vec_256(t_vals);
  printf("\n");
};

void test_update_sphere_cluster() {
  printf("TESTING UPDATE_SPHERE_CLUSTER\n");
  SphereCluster sphere_cluster{
      .center = {.x = _mm256_setzero_ps(), .y = _mm256_setzero_ps(), .z = _mm256_setzero_ps()},
      .r = _mm256_setzero_ps(),
  };
  const Sphere rando_sphere1{
      .center = {.x = 69.f, .y = 420.f, .z = 21.f},
      .r = .8f,
  };

  const Sphere rando_sphere2{
      .center = {.x = 100.f, .y = -2.3f, .z = 800.f},
      .r = 0.39f,
  };

  uint32_t a = 0xFFFFFFFF;
  float set = *((float*)&a);

  __m256 mask = _mm256_set_ps(set, 0.f, set, 0.f, set, 0.f, set, 0.f);
  update_sphere_cluster(&sphere_cluster, rando_sphere1, mask);

  printf("iter 1\n");
  print_vec_256(sphere_cluster.center.x);
  print_vec_256(sphere_cluster.center.y);
  print_vec_256(sphere_cluster.center.z);
  print_vec_256(sphere_cluster.r);

  mask = _mm256_set_ps(0.f, 0.f, 0.f, set, set, set, set, 0.f);
  update_sphere_cluster(&sphere_cluster, rando_sphere2, mask);

  printf("iter 2\n");
  print_vec_256(sphere_cluster.center.x);
  print_vec_256(sphere_cluster.center.y);
  print_vec_256(sphere_cluster.center.z);
  print_vec_256(sphere_cluster.r);

  printf("\n");
}

void test_reflect() {
  printf("TESTING REFLECT\n");
  Vec3_256 axis = {
      .x = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
      .y = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
      .z = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
  };

  Vec3_256 ray_dirs = rays.dir;

  reflect(&ray_dirs, &axis);

  print_vec_256(ray_dirs.x);
  print_vec_256(ray_dirs.y);
  print_vec_256(ray_dirs.z);
  printf("\n");
}

void test_dot() {
  printf("TESTING DOT PRODUCT\n");

  __m256 dp = dot(&rays.dir, &rays.dir);

  print_vec_256(dp);
  printf("\n");
}

void test_normalize() {
  printf("TESTING NORMALIZE\n");

  Vec3_256 ray_dirs = rays.dir;

  normalize(&ray_dirs);

  print_vec_256(ray_dirs.x);
  print_vec_256(ray_dirs.y);
  print_vec_256(ray_dirs.z);
  printf("\n");
}

void test_refract() {
  printf("TESTING NORMALIZE\n");
  Vec3 ray_dir = {0.0014, 0.2775, -10};
  Vec3_256 ray_dir_vec = broadcast_vec(&ray_dir);
  Vec3 norm = {0.003, 0.6133, 0.790};
  Vec3_256 norm_vec = broadcast_vec(&norm);
  Vec3_256 refracted = refract(&ray_dir_vec, &norm_vec, global::rcp_ir_vec);
  printf("\n");
}

int main() {
  // test_sphere_hit();
  // test_update_sphere_cluster();
  // test_reflect();
  // test_dot();
  // test_normalize();
  test_refract();
  return 0;
}
