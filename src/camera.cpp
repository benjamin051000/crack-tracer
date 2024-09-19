#include "camera.hpp"

void Camera::register_key_event(SDL_Event e) {
  uint32_t key = e.key.keysym.sym;

  if (e.type == SDL_KEYDOWN) {
    if (key == SDLK_w) {
      velocity.z = -speed;
    }
    if (key == SDLK_s) {
      velocity.z = speed;
    }
    if (key == SDLK_a) {
      velocity.x = -speed;
    }
    if (key == SDLK_d) {
      velocity.x = speed;
    }
  }
  if (e.type == SDL_KEYUP) {
    if (key == SDLK_w) {
      velocity.z = 0;
    }
    if (key == SDLK_s) {
      velocity.z = 0;
    }
    if (key == SDLK_a) {
      velocity.x = 0;
    }
    if (key == SDLK_d) {
      velocity.x = 0;
    }
  }
}

void Camera::update() {
  origin.x += velocity.x;
  origin.y += velocity.y;
  origin.z += velocity.z;
}
