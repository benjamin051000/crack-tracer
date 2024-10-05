#include "camera.hpp"

void Camera::register_key_event(const SDL_Event e) {
  const auto key = e.key.keysym.sym;

  if (e.type == SDL_KEYDOWN) {
    switch (key) {
    case SDLK_w:
      velocity.z = -speed;
      break;
    case SDLK_s:
      velocity.z = speed;
      break;
    case SDLK_a:
      velocity.x = -speed;
      break;
    case SDLK_d:
      velocity.x = speed;
      break;
    default:
      break;
    }
  }
  else if (e.type == SDL_KEYUP) {
    switch (key) {
    case SDLK_w:
    case SDLK_s:
      velocity.z = 0;
      break;
    case SDLK_a:
    case SDLK_d:
      velocity.x = 0;
      break;
    default:
      break;
    }
  }
}

void Camera::update() {
	// TODO origin += velocity
  origin.x += velocity.x;
  origin.y += velocity.y;
  origin.z += velocity.z;
}
