#pragma once
#include "camera.hpp"
#include "comptime.hpp"
#include "globals.hpp"
#include "materials.hpp"
#include "sphere.hpp"
#include "types.hpp"
#include <SDL2/SDL.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <future>
#include <immintrin.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

inline static void update_colors(Color_256* curr_colors, const Color_256* new_colors,
                                 __m256 update_mask) {

  __m256 new_no_hit_mask = _mm256_xor_ps(update_mask, (__m256)global::all_set);
  __m256 preserve_curr = _mm256_and_ps(global::ones, new_no_hit_mask);

  // multiply current colors by the attenuation of new hits.
  // fill 1.0 for no hits in order to preserve current colors when multiplying
  *curr_colors *= ((*new_colors & update_mask) + preserve_curr);
}

inline static Color_256 ray_cluster_colors(RayCluster* rays) {
  // will be used to add a sky tint to rays that at some point bounce off into space.
  // if a ray never bounces away (within amount of bounces set by depth), the
  // hit_mask will be all set (packed floats) and the sky tint will not affect its final color
  __m256 no_hit_mask = global::zeros;

  HitRecords hit_rec;
  hit_rec.front_face = global::zeros;

  Color_256 colors{
      .x = global::ones,
      .y = global::ones,
      .z = global::ones,
  };

  for (int i = 0; i < global::ray_depth; i++) {

    find_sphere_hits(&hit_rec, rays, INFINITY);

    // or a mask when a value is not a hit, at any point.
    // if all are zero, break
    __m256 new_hit_mask = _mm256_cmp_ps(hit_rec.t, global::zeros, _CMP_NLE_US);
    __m256 new_no_hit_mask = _mm256_xor_ps(new_hit_mask, (__m256)global::all_set);

    no_hit_mask = _mm256_or_ps(no_hit_mask, new_no_hit_mask);
    if (_mm256_testz_ps(new_hit_mask, new_hit_mask)) {
      update_colors(&colors, &background_color, no_hit_mask);
      break;
    }

    scatter(rays, &hit_rec);

    update_colors(&colors, &hit_rec.mat.atten, new_hit_mask);
  }

  return colors;
};

// writes a color buffer of 32 Color values to an image buffer
// uses non temporal writes to avoid filling data cache
inline static void write_out_color_buf(const Color* color_buf, CharColor* img_buf,
                                       uint32_t write_pos) {

  __m256 cm = _mm256_broadcast_ss(&global::color_multiplier);
  __m256 colors_1_f32 = _mm256_load_ps((float*)color_buf) * cm;
  __m256 colors_2_f32 = _mm256_load_ps((float*)(color_buf) + 8) * cm;
  __m256 colors_3_f32 = _mm256_load_ps((float*)(color_buf) + 16) * cm;
  __m256 colors_4_f32 = _mm256_load_ps((float*)(color_buf) + 24) * cm;
  __m256 colors_5_f32 = _mm256_load_ps((float*)(color_buf) + 32) * cm;
  __m256 colors_6_f32 = _mm256_load_ps((float*)(color_buf) + 40) * cm;
  __m256 colors_7_f32 = _mm256_load_ps((float*)(color_buf) + 48) * cm;
  __m256 colors_8_f32 = _mm256_load_ps((float*)(color_buf) + 56) * cm;
  __m256 colors_9_f32 = _mm256_load_ps((float*)(color_buf) + 64) * cm;
  __m256 colors_10_f32 = _mm256_load_ps((float*)(color_buf) + 72) * cm;
  __m256 colors_11_f32 = _mm256_load_ps((float*)(color_buf) + 80) * cm;
  __m256 colors_12_f32 = _mm256_load_ps((float*)(color_buf) + 88) * cm;

  __m256i colors_1_i32 = _mm256_cvtps_epi32(colors_1_f32);
  __m256i colors_2_i32 = _mm256_cvtps_epi32(colors_2_f32);
  __m256i colors_3_i32 = _mm256_cvtps_epi32(colors_3_f32);
  __m256i colors_4_i32 = _mm256_cvtps_epi32(colors_4_f32);
  __m256i colors_5_i32 = _mm256_cvtps_epi32(colors_5_f32);
  __m256i colors_6_i32 = _mm256_cvtps_epi32(colors_6_f32);
  __m256i colors_7_i32 = _mm256_cvtps_epi32(colors_7_f32);
  __m256i colors_8_i32 = _mm256_cvtps_epi32(colors_8_f32);
  __m256i colors_9_i32 = _mm256_cvtps_epi32(colors_9_f32);
  __m256i colors_10_i32 = _mm256_cvtps_epi32(colors_10_f32);
  __m256i colors_11_i32 = _mm256_cvtps_epi32(colors_11_f32);
  __m256i colors_12_i32 = _mm256_cvtps_epi32(colors_12_f32);

  const uint8_t BOTH_LOW_XMMWORD = 32;
  const uint8_t BOTH_HIGH_XMMWORD = 49;
  __m256i temp_permute_1 = _mm256_permute2x128_si256(colors_1_i32, colors_2_i32, BOTH_LOW_XMMWORD);
  __m256i temp_permute_2 = _mm256_permute2x128_si256(colors_1_i32, colors_2_i32, BOTH_HIGH_XMMWORD);
  __m256i temp_permute_3 = _mm256_permute2x128_si256(colors_3_i32, colors_4_i32, BOTH_LOW_XMMWORD);
  __m256i temp_permute_4 = _mm256_permute2x128_si256(colors_3_i32, colors_4_i32, BOTH_HIGH_XMMWORD);
  __m256i temp_permute_5 = _mm256_permute2x128_si256(colors_5_i32, colors_6_i32, BOTH_LOW_XMMWORD);
  __m256i temp_permute_6 = _mm256_permute2x128_si256(colors_5_i32, colors_6_i32, BOTH_HIGH_XMMWORD);
  __m256i temp_permute_7 = _mm256_permute2x128_si256(colors_7_i32, colors_8_i32, BOTH_LOW_XMMWORD);
  __m256i temp_permute_8 = _mm256_permute2x128_si256(colors_7_i32, colors_8_i32, BOTH_HIGH_XMMWORD);
  __m256i temp_permute_9 = _mm256_permute2x128_si256(colors_9_i32, colors_10_i32, BOTH_LOW_XMMWORD);
  __m256i temp_permute_10 =
      _mm256_permute2x128_si256(colors_9_i32, colors_10_i32, BOTH_HIGH_XMMWORD);
  __m256i temp_permute_11 =
      _mm256_permute2x128_si256(colors_11_i32, colors_12_i32, BOTH_LOW_XMMWORD);
  __m256i temp_permute_12 =
      _mm256_permute2x128_si256(colors_11_i32, colors_12_i32, BOTH_HIGH_XMMWORD);

  __m256i colors_1_i16 = _mm256_packs_epi32(temp_permute_1, temp_permute_2);
  __m256i colors_2_i16 = _mm256_packs_epi32(temp_permute_3, temp_permute_4);
  __m256i colors_3_i16 = _mm256_packs_epi32(temp_permute_5, temp_permute_6);
  __m256i colors_4_i16 = _mm256_packs_epi32(temp_permute_7, temp_permute_8);
  __m256i colors_5_i16 = _mm256_packs_epi32(temp_permute_9, temp_permute_10);
  __m256i colors_6_i16 = _mm256_packs_epi32(temp_permute_11, temp_permute_12);

  temp_permute_1 = _mm256_permute2x128_si256(colors_1_i16, colors_2_i16, BOTH_LOW_XMMWORD);
  temp_permute_2 = _mm256_permute2x128_si256(colors_1_i16, colors_2_i16, BOTH_HIGH_XMMWORD);
  temp_permute_3 = _mm256_permute2x128_si256(colors_3_i16, colors_4_i16, BOTH_LOW_XMMWORD);
  temp_permute_4 = _mm256_permute2x128_si256(colors_3_i16, colors_4_i16, BOTH_HIGH_XMMWORD);
  temp_permute_5 = _mm256_permute2x128_si256(colors_5_i16, colors_6_i16, BOTH_LOW_XMMWORD);
  temp_permute_6 = _mm256_permute2x128_si256(colors_5_i16, colors_6_i16, BOTH_HIGH_XMMWORD);

  __m256i colors_1_u8 = _mm256_packus_epi16(temp_permute_1, temp_permute_2);
  __m256i colors_2_u8 = _mm256_packus_epi16(temp_permute_3, temp_permute_4);
  __m256i colors_3_u8 = _mm256_packus_epi16(temp_permute_5, temp_permute_6);

  // SDL offsets our img pointer to a location that might not be aligned to 32 bytes.
  // Therefore we can't just stream from the registers to memory... :(
  write_pos *= 3;
  if constexpr (global::active_render_mode == RenderMode::real_time) {
    alignas(32) CharColor char_buf[32];
    _mm256_store_si256(((__m256i*)char_buf), colors_1_u8);
    _mm256_store_si256(((__m256i*)char_buf) + 1, colors_2_u8);
    _mm256_store_si256(((__m256i*)char_buf) + 2, colors_3_u8);

    int* img_ints = (int*)(((__m256i*)img_buf) + write_pos);
    int* color_ints = (int*)(char_buf);

    for (int i = 0; i < 24; i += 4) {
      _mm_stream_si32((img_ints + i), *(color_ints + i));
      _mm_stream_si32((img_ints + i + 1), *(color_ints + i + 1));
      _mm_stream_si32((img_ints + i + 2), *(color_ints + i + 2));
      _mm_stream_si32((img_ints + i + 3), *(color_ints + i + 3));
    }
  } else {
    _mm256_stream_si256(((__m256i*)img_buf) + write_pos, colors_1_u8);
    _mm256_stream_si256(((__m256i*)img_buf) + write_pos + 1, colors_2_u8);
    _mm256_stream_si256(((__m256i*)img_buf) + write_pos + 2, colors_3_u8);
  }
}

inline static void render(CharColor* img_buf, const Vec3 cam_origin, uint32_t pix_offset) {
  // comptime generated
  constexpr Vec3_256 base_dirs = comptime::init_ray_directions();
  RayCluster base_rays = {
      .dir = base_dirs,
      .orig = broadcast_vec(&cam_origin),
  };

  Color_256 sample_color;
  alignas(32) Color color_buf[32];

  constexpr uint32_t write_chunk_size = global::img_width / 32;
  uint32_t row = pix_offset / global::img_width;
  uint32_t write_pos = row * write_chunk_size;
  uint16_t color_buf_idx = 0;
  uint16_t sample_group;

  static_assert(global::sample_group_num > 0,
                "there must be at least one group of ray samples to calculate");

  for (; row < global::img_height; row += global::thread_count) {
    for (uint32_t col = 0; col < global::img_width; col++) {
      sample_color.x = _mm256_setzero_ps();
      sample_color.y = _mm256_setzero_ps();
      sample_color.z = _mm256_setzero_ps();

      for (sample_group = 0; sample_group < global::sample_group_num; sample_group++) {
        RayCluster samples = base_rays;

        float x_scale = global::pix_du * col;
        __m256 x_scale_vec = _mm256_broadcast_ss(&x_scale);
        samples.dir.x = samples.dir.x + x_scale_vec;

        float y_scale = (global::pix_dv * row) + (sample_group * global::sample_dv);
        __m256 y_scale_vec = _mm256_broadcast_ss(&y_scale);
        samples.dir.y += y_scale_vec;

        sample_color += ray_cluster_colors(&samples);
      }

      // accumulate all color channels into first float of vec
      sample_color.x = _mm256_hadd_ps(sample_color.x, sample_color.x);
      sample_color.x = _mm256_hadd_ps(sample_color.x, sample_color.x);
      sample_color.x = _mm256_hadd_ps(sample_color.x, sample_color.x);

      sample_color.y = _mm256_hadd_ps(sample_color.y, sample_color.y);
      sample_color.y = _mm256_hadd_ps(sample_color.y, sample_color.y);
      sample_color.y = _mm256_hadd_ps(sample_color.y, sample_color.y);

      sample_color.z = _mm256_hadd_ps(sample_color.z, sample_color.z);
      sample_color.z = _mm256_hadd_ps(sample_color.z, sample_color.z);
      sample_color.z = _mm256_hadd_ps(sample_color.z, sample_color.z);

      _mm_store_ss(&color_buf[color_buf_idx].x, _mm256_castps256_ps128(sample_color.x));
      _mm_store_ss(&color_buf[color_buf_idx].y, _mm256_castps256_ps128(sample_color.y));
      _mm_store_ss(&color_buf[color_buf_idx].z, _mm256_castps256_ps128(sample_color.z));

      color_buf_idx++;

      if (color_buf_idx != 32) {
        continue;
      }

      write_out_color_buf(color_buf, img_buf, write_pos);
      write_pos++;

      color_buf_idx = 0;
    }
    write_pos += ((global::thread_count - 1) * write_chunk_size);
  }
}

using namespace std::chrono;

inline static void render_png() {
  static_assert(global::img_height % global::thread_count == 0,
                "Thread count must divide rows equally");

  CharColor* img_data =
      (CharColor*)aligned_alloc(32, global::img_width * global::img_height * sizeof(CharColor));
  init_spheres();
  std::array<std::future<void>, global::thread_count> futures;
  Camera cam;

  auto start_time = system_clock::now();

  for (size_t idx = 0; idx < global::thread_count; idx++) {
    futures[idx] =
        std::async(std::launch::async, render, img_data, cam.origin, idx * global::img_width);
  }

  for (size_t idx = 0; idx < global::thread_count; idx++) {
    futures[idx].get();
  }

  auto end_time = system_clock::now();
  auto dur = duration<float>(end_time - start_time);
  float milli = duration_cast<microseconds>(dur).count() / 1000.f;
  printf("render time (ms): %f\n", milli);
  stbi_write_png("out.png", global::img_width, global::img_height, 3, img_data,
                 global::img_width * sizeof(CharColor));
}

inline static void render_realtime() {
  static_assert(global::img_height % global::thread_count == 0,
                "Thread count must divide rows equally");
  CharColor* img_data =
      (CharColor*)aligned_alloc(32, global::img_width * global::img_height * sizeof(CharColor));
  init_spheres();
  std::array<std::future<void>, global::thread_count> futures{};
  Camera cam;

  SDL_Window* win = NULL;
  SDL_Renderer* renderer = NULL;

  int sdl_res = SDL_Init(SDL_INIT_VIDEO);

  if (sdl_res < 0) {
    printf("SDL initialization failed with status code: %d\n", sdl_res);
    exit(EXIT_FAILURE);
  }

  win = SDL_CreateWindow("Crack Tracer", 100, 100, global::img_width, global::img_height, 0);
  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  SDL_Texture* buffer =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                        global::img_width, global::img_height);

  int pitch = global::img_width * sizeof(CharColor);

  while (true) {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        break;
      }
      cam.register_key_event(e);
    }

    cam.update();

    SDL_LockTexture(buffer, NULL, (void**)(&img_data), &pitch);

    for (size_t idx = 0; idx < global::thread_count; idx++) {
      futures[idx] =
          std::async(std::launch::async, render, img_data, cam.origin, idx * global::img_width);
    }

    for (size_t idx = 0; idx < global::thread_count; idx++) {
      futures[idx].get();
    }

    SDL_UnlockTexture(buffer);

    SDL_RenderCopy(renderer, buffer, NULL, NULL);

    // flip the backbuffer
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(buffer);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(win);
}
