#pragma once

#include <SDL2/SDL_rect.h>

class Pixel {
public:
  SDL_FPoint pos;
  SDL_Color col;

  Pixel(SDL_FPoint pos, Uint32 col) {
    this->pos = pos;
    this->col = {
        static_cast<Uint8>((col >> 24) & 0xff),
        static_cast<Uint8>((col >> 16) & 0xff),
        static_cast<Uint8>((col >> 8) & 0xff),
        static_cast<Uint8>((col >> 0) & 0xff),
    };
  }
};
