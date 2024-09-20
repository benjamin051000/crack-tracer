#pragma once
#include "rand.hpp"
#include "types.hpp"
#include <cmath>
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

// TODO do these need to be global? Or can they just chill in the cpp file?
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

// static LCGRand lcg_rand;
void scatter_metallic(RayCluster& rays, const HitRecords& hit_rec);

// TODO move into the math area probably
[[nodiscard]] __m256 near_zero(const Vec3_256& vec);

void scatter_lambertian(RayCluster& rays, const HitRecords& hit_rec);

[[nodiscard]] __m256 reflectance(const __m256 cos, const __m256 ref_idx);

void scatter_dielectric(RayCluster& rays, const HitRecords& hit_rec);

void scatter(RayCluster& rays, const HitRecords& hit_rec);
