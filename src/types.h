#pragma once
#include <immintrin.h>

struct Color {
  float r;
  float g;
  float b;
};

struct Colors {
  __m256 r;
  __m256 g;
  __m256 b;
};

struct alignas(16) Sphere {
  float x;
  float y;
  float z;
  float r;
};

struct Material {
  Colors attenuation;
};

struct SphereCluster {
  __m256 x;
  __m256 y;
  __m256 z;
  __m256 r;
  __m256i mat_idx;
};

struct RayCluster {
  __m256 dir_x;
  __m256 dir_y;
  __m256 dir_z;
};

struct HitRecords {
  __m256 t;
  __m256 orig_x;
  __m256 orig_y;
  __m256 orig_z;
  __m256 norm_x;
  __m256 norm_y;
  __m256 norm_z;
  __m256i mat_idx;
};
