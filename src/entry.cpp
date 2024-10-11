#include "camera.hpp"
#include "globals.hpp"
#include "render.hpp"
#include <chrono>
#include <future>

void render_png() {
  using namespace std::chrono;

  CharColor* const img_data = static_cast<CharColor*>(
      aligned_alloc(32, config::img_width * config::img_height * sizeof(CharColor)));
  init_spheres();
  std::array<std::future<void>, config::thread_count> futures;
  Camera cam;

  const auto start_time = system_clock::now();

  for (size_t idx = 0; idx < config::thread_count; idx++) {
    futures[idx] =
        std::async(std::launch::async, render, img_data, cam.origin, idx * config::img_width);
  }

  for (size_t idx = 0; idx < config::thread_count; idx++) {
    futures[idx].get();
  }

  const auto end_time = system_clock::now();
  const auto dur = duration<float>(end_time - start_time);
  const float milli = static_cast<float>(duration_cast<microseconds>(dur).count()) / 1000.f;
  printf("render time (ms): %f\n", milli);

  stbi_write_png("out.png", config::img_width, config::img_height, 3, img_data,
                 config::img_width * sizeof(CharColor));
}

void render_realtime() {
  CharColor* img_data =
      (CharColor*)aligned_alloc(32, config::img_width * config::img_height * sizeof(CharColor));
  init_spheres();
  std::array<std::future<void>, config::thread_count> futures{};
  Camera cam;

  SDL_Window* win = NULL;
  SDL_Renderer* renderer = NULL;

  int sdl_res = SDL_Init(SDL_INIT_VIDEO);

  if (sdl_res < 0) {
    printf("SDL initialization failed with status code: %d\n", sdl_res);
    exit(EXIT_FAILURE);
  }

  win = SDL_CreateWindow("Crack Tracer", 100, 100, config::img_width, config::img_height, 0);
  renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

  SDL_Texture* buffer =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
                        config::img_width, config::img_height);

  int pitch = config::img_width * sizeof(CharColor);

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

    for (size_t idx = 0; idx < config::thread_count; idx++) {
      futures[idx] =
          std::async(std::launch::async, render, img_data, cam.origin, idx * config::img_width);
    }

    for (size_t idx = 0; idx < config::thread_count; idx++) {
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

int main() {
  if constexpr (config::render_mode == RenderMode::real_time) {
    render_realtime();
  } else if constexpr (config::render_mode == RenderMode::png) {
    render_png();
  }
  return 0;
}
