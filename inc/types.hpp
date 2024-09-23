#pragma once
#include "colors.hpp"
#include "vec.hpp"
#include <immintrin.h>

using namespace colors;

enum MatType {
  metallic,
  lambertian,
  dielectric,
};

struct Material_256 {
  Color_256 atten;
  __m256i type;
};

struct alignas(16) Material {
  Color atten;
  MatType type;
};

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

struct RayCluster {
  Vec3_256 dir;
  Vec3_256 orig;
};

struct HitRecords {
  Vec3_256 orig;
  Vec3_256 norm;
  Material_256 mat;
  __m256 front_face;
  __m256 t;
};
