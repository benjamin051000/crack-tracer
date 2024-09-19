#pragma once
#include <types.hpp>

constexpr Color_256 sky = {
    .x = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f},
    .y = {0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f, 0.7f},
    .z = {1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f},
};

void update_colors(Color_256* curr_colors, const Color_256* new_colors,
                                 __m256 update_mask);

 Color_256 ray_cluster_colors(RayCluster* rays) ;

// writes a color buffer of 32 Color values to an image buffer
// uses non temporal writes to avoid filling data cache
 void write_out_color_buf(const Color* color_buf, CharColor* img_buf,
                                       uint32_t write_pos);

 void render(CharColor* img_buf, const Vec3 cam_origin, uint32_t pix_offset);

 void render_png();

 void render_realtime();
