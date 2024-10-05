#pragma once
#include "globals.hpp"
#include "vec.hpp"

using Color = Vec3;
using Color_256 = Vec3_256;

namespace colors {

constexpr Color silver = {.x = 0.5f, .y = 0.5f, .z = 0.5f};
constexpr Color grey = {.x = 0.5f, .y = 0.5f, .z = 0.5f};
constexpr Color white = {.x = 1.f, .y = 1.f, .z = 1.f};
constexpr Color red = {.x = 0.90f, .y = 0.20f, .z = 0.20f};
constexpr Color gold = {.x = 0.90f, .y = 0.75f, .z = 0.54f};
constexpr Color copper = {.x = 0.59f, .y = 0.34f, .z = 0.29f};
constexpr Color green = {.x = 0.f, .y = 1.f, .z = 0.f};
constexpr Color moon = {.x = 100.f, .y = 100.f, .z = 100.f};

constexpr Color_256 night = {
    .x = {0.02f, 0.02f, 0.02f, 0.02f, 0.02f, 0.02f, 0.02f, 0.02f},
    .y = {0.08f, 0.08f, 0.08f, 0.08f, 0.08f, 0.08f, 0.08f, 0.08f},
    .z = {0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f, 0.35f},
};

constexpr Color_256 sky = {
    .x = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f},
    .y = {0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f},
    .z = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
};

constexpr Color_256 background_color = {.x = global::ones, .y = global::ones, .z = global::ones};
} // end of namespace colors
