#pragma once
#include "vec.hpp"
#include <SDL_events.h>
#include <SDL_keycode.h>

class Camera {
public:
  Vec3 origin{-1.2f, 1.f, 5.f};

  void register_key_event(const SDL_Event e);
  void update();

private:
  Vec3 velocity{0, 0, 0};
  static constexpr float speed = 0.01f;
};
