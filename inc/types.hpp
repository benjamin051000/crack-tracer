#pragma once
#include <cstdint>
#include <immintrin.h>

struct Vec3 {
  float x, y, z;
};

struct Vec3_256 {
  __m256 x, y, z;
};

struct CharColor {
  uint8_t r, g, b;
};

enum MatType {
  metallic,
  lambertian,
  dielectric,
};

using Color_256 = Vec3_256;
using Color = Vec3;

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
