#include "render.h"

void scatter_metallic(RayCluster* rays, const HitRecords* hit_rec) {
  // reflect ray direction about the hit record normal
  reflect(&rays->dir, &hit_rec->norm);
  normalize(&rays->dir);
};

void update_colors(Color_256* curr_colors, const Color_256* new_colors, __m256 update_mask) {

  // multiply current colors by the attenuation of new hits.
  // fill 1.0 for no hits in order to preserve current colors when multiplying
  __m256 r = _mm256_and_ps(new_colors->r, update_mask);
  __m256 g = _mm256_and_ps(new_colors->g, update_mask);
  __m256 b = _mm256_and_ps(new_colors->b, update_mask);

  __m256 new_no_hit_mask = _mm256_xor_ps(update_mask, global::all_set);
  __m256 preserve_curr = _mm256_and_ps(global::white, new_no_hit_mask);

  // 1.0 where no new hit occured and we want to preserve the prev color.
  // new r,g,b values where a new hit occured and we want to scale prev color by
  r = _mm256_add_ps(r, preserve_curr);
  g = _mm256_add_ps(g, preserve_curr);
  b = _mm256_add_ps(b, preserve_curr);

  curr_colors->r = _mm256_mul_ps(curr_colors->r, r);
  curr_colors->g = _mm256_mul_ps(curr_colors->g, g);
  curr_colors->b = _mm256_mul_ps(curr_colors->b, b);
}

Color_256 ray_cluster_colors(RayCluster* rays, const Sphere* spheres, uint8_t depth) {
  const __m256 zeros = _mm256_setzero_ps();
  // will be used to add a sky tint to rays that at some point bounce off into space.
  // if a ray never bounces away (within amount of bounces set by depth), the
  // hit_mask will be all set (packed floats) and the sky tint will not affect its final color
  __m256 no_hit_mask = zeros;
  HitRecords hit_rec;
  SphereCluster closest_spheres = {
      .center =
          {
              .x = zeros,
              .y = zeros,
              .z = zeros,
          },
      .r = zeros,
  };
  Color_256 colors{
      .r = global::white,
      .g = global::white,
      .b = global::white,
  };

  for (int i = 0; i < depth; i++) {
    closest_spheres.center.x = zeros;
    closest_spheres.center.y = zeros;
    closest_spheres.center.z = zeros;
    closest_spheres.r = zeros;

    find_sphere_hits(&closest_spheres, &hit_rec, spheres, rays, INFINITY);

    __m256 new_hit_mask = _mm256_cmp_ps(hit_rec.t, zeros, CMPNLE);
    // or a mask when a value is not a hit, at any point. if all are zero,
    // break
    __m256 new_no_hit_mask = _mm256_xor_ps(new_hit_mask, global::all_set);
    no_hit_mask = _mm256_or_ps(no_hit_mask, new_no_hit_mask);
    if (_mm256_testz_ps(new_hit_mask, new_hit_mask)) {
      break;
    }

    scatter_metallic(rays, &hit_rec);
    update_colors(&colors, &hit_rec.mat.atten, new_hit_mask);
  }

  update_colors(&colors, &sky, no_hit_mask);

  return colors;
};

void render(CharColor* data, uint32_t pixel_count, uint32_t offset) {
  constexpr RayCluster base_dirs = generate_init_directions();

  Color_256 sample_color;
  Color final_color;
  CharColor char_color;
  uint32_t write_pos;
  uint16_t sample_group;
  uint32_t row = offset / IMG_WIDTH;
  uint32_t col = offset % IMG_WIDTH;
  uint32_t end_row = (offset + pixel_count - 1) / IMG_WIDTH;
  uint32_t end_col = (offset + pixel_count - 1) % IMG_WIDTH;

  for (; row <= end_row; row++) {
    while (col <= end_col) {
      sample_color.r = _mm256_setzero_ps();
      sample_color.g = _mm256_setzero_ps();
      sample_color.b = _mm256_setzero_ps();
      // 64 samples per pixel. 8 rays calculated 8 times
      // if (row > IMG_HEIGHT / 2 && col > IMG_WIDTH / 2) {
      //  BREAKPOINT
      //}
      for (sample_group = 0; sample_group < 8; sample_group++) {

        RayCluster samples = base_dirs;
        float x_scale = PIX_DU * col;
        float y_scale = (PIX_DV * row) + (sample_group * SAMPLE_DV);

        __m256 x_scale_vec = _mm256_broadcast_ss(&x_scale);
        __m256 y_scale_vec = _mm256_broadcast_ss(&y_scale);

        samples.dir.x = _mm256_add_ps(samples.dir.x, x_scale_vec);
        samples.dir.y = _mm256_add_ps(samples.dir.y, y_scale_vec);

        Color_256 new_colors = ray_cluster_colors(&samples, spheres, 10);
        sample_color.r = _mm256_add_ps(sample_color.r, new_colors.r);
        sample_color.g = _mm256_add_ps(sample_color.g, new_colors.g);
        sample_color.b = _mm256_add_ps(sample_color.b, new_colors.b);
      }
      // accumulate all color channels into first float of vec
      sample_color.r = _mm256_hadd_ps(sample_color.r, sample_color.r);
      sample_color.r = _mm256_hadd_ps(sample_color.r, sample_color.r);
      sample_color.r = _mm256_hadd_ps(sample_color.r, sample_color.r);

      sample_color.g = _mm256_hadd_ps(sample_color.g, sample_color.g);
      sample_color.g = _mm256_hadd_ps(sample_color.g, sample_color.g);
      sample_color.g = _mm256_hadd_ps(sample_color.g, sample_color.g);

      sample_color.b = _mm256_hadd_ps(sample_color.b, sample_color.b);
      sample_color.b = _mm256_hadd_ps(sample_color.b, sample_color.b);
      sample_color.b = _mm256_hadd_ps(sample_color.b, sample_color.b);

      _mm_store_ss(&final_color.r, _mm256_castps256_ps128(sample_color.r));
      _mm_store_ss(&final_color.g, _mm256_castps256_ps128(sample_color.g));
      _mm_store_ss(&final_color.b, _mm256_castps256_ps128(sample_color.b));

      // average by sample count. color / 64
      char_color.r = final_color.r * COLOR_MULTIPLIER;
      char_color.g = final_color.g * COLOR_MULTIPLIER;
      char_color.b = final_color.b * COLOR_MULTIPLIER;

      write_pos = col + row * IMG_WIDTH;
      data[write_pos] = char_color;
      col++;
    }
    col = 0;
  }
}
