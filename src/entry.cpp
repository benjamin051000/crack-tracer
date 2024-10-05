#include "globals.hpp"
#include "render.hpp"

int main() {
  if constexpr (config::render_mode == RenderMode::real_time) {
    render_realtime();
  } else if constexpr (config::render_mode == RenderMode::png) {
    render_png();
  }
}
