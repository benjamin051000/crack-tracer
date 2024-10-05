#pragma once
#include <immintrin.h>
#include <cfloat>
#include <cstdint>

enum RenderMode {
  png,
  real_time,
};

namespace global {
  constexpr uint16_t img_width = 1920;
  constexpr uint16_t img_height = 1080;
  constexpr uint8_t thread_count = 12;
  constexpr RenderMode active_render_mode = RenderMode::png;

  // each group calculates 8 samples.
  constexpr uint16_t sample_group_num = 10;
  constexpr uint8_t ray_depth = 20;
  constexpr float float_max = FLT_MAX; // TODO use limits

  constexpr float viewport_height = 2.f;
  constexpr float viewport_width = viewport_height * (float(img_width) / img_height);
  constexpr float pix_du = viewport_width / img_width;
  constexpr float pix_dv = -viewport_height / img_height;
  // 8 evenly spread out ray directions. (space-around)
  constexpr float sample_du = pix_du / 9;
  constexpr float sample_dv = pix_dv / (sample_group_num + 1);
  constexpr float focal_len = 1.0; // TODO move to camera?
  constexpr float color_multiplier = 255.f / (sample_group_num * 8);

  // index of refraction
  constexpr float ir = 1.5;
  // constexpr float ir = 1.5;
  constexpr __m256 ir_vec = {ir, ir, ir, ir, ir, ir, ir, ir};
  constexpr float rcp_ir = 1.f / ir;
  constexpr __m256 rcp_ir_vec = {rcp_ir, rcp_ir, rcp_ir, rcp_ir, rcp_ir, rcp_ir, rcp_ir, rcp_ir};
  alignas(32) constexpr float cam_origin[4] = {0.f, 0.f, 0.0f, 0.f};
  constexpr float t_min = 0.0013f;
  constexpr __m256 t_min_vec = {t_min, t_min, t_min, t_min, t_min, t_min, t_min, t_min};

constexpr __m256 zeros = {0, 0, 0, 0, 0, 0, 0, 0};
constexpr __m256 ones = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f};

const __m256i all_set = (__m256i)_mm256_cmp_ps(_mm256_setzero_ps(), _mm256_setzero_ps(), _CMP_EQ_OQ); // TODO replace with the predefined ones (cmpeq)
} // namespace global

namespace { // simply to remove the need for `static` on all these methods.

// TODO move to globals.cpp? Also, is this really necessary? Or is std stoi or whatever it is fast enough?
[[nodiscard, gnu::always_inline]] inline uint32_t f_to_i(float& f_val) {
  f_val += 1 << 23;
  return ((uint32_t)f_val) & 0x007FFFFF; // TODO use a safer cast... bit_cast?
}

} // end of namespace (anonymous)
