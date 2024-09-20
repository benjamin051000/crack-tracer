#pragma once
#include <cstdint>
#include <immintrin.h>

struct Vec3 {
  float x;
  float y;
  float z;
};

struct Vec3_256 {
  __m256 x;
  __m256 y;
  __m256 z;
};

struct CharColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

enum MatType {
  metallic,
  lambertian,
  dielectric,
};

using Color_256 = Vec3_256;
struct Material_256 {
  Color_256 atten;
  __m256i type;
};

using Color = Vec3;
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
