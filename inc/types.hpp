#pragma once
#include "colors.hpp"
#include "vec.hpp"
#include <immintrin.h>

struct RayCluster {
  Vec3_256 dir;
  Vec3_256 orig;
};

struct Material_256 {
  Color_256 atten;
  __m256i type;
};

struct HitRecords {
  Vec3_256 orig;
  Vec3_256 norm;
  Material_256 mat;
  __m256 front_face;
  __m256 t;
};
