#include "failure.hpp"
#include "input.hpp"
#include "pixel.hpp"
#include "vec3d.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <iostream>
#include <vector>

class Display {
  SDL_Event event;
  SDL_Window *window;
  SDL_Renderer *renderer;

  std::vector<Pixel *> pixels;

public:
  int width;
  int height;
  Display(int width, int height) {
    this->width = width;
    this->height = height;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      fail("Could not initialize SDL2");
    }
    if (SDL_CreateWindowAndRenderer(this->width, this->height, 0,
                                    &(this->window), &(this->renderer)) < 0) {
      fail("Could not create window and renderer");
    }
  }
  Display() { *this = Display(1280, 720); }

  void clear() { this->pixels.clear(); }

  void pixel(SDL_FPoint point, Uint32 color) {
    if (point.x < 0 || point.x >= this->width || point.y < 0 ||
        point.y >= this->height) {
    } else {
      this->pixels.push_back(new Pixel(point, color));
    }
    // std::cout << this->pixels.size() << std::endl;
  }
  void pixel(float x, float y, Uint32 color) { this->pixel({x, y}, color); }
  void line(float x1, float y1, float x2, float y2, Uint32 color) {
    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = abs(dx);
    dy1 = abs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;
    if (dy1 <= dx1) {
      if (dx >= 0) {
        x = x1;
        y = y1;
        xe = x2;
      } else {
        x = x2;
        y = y2;
        xe = x1;
      }

      this->pixel(x, y, color);

      for (i = 0; x < xe; i++) {
        x = x + 1;
        if (px < 0)
          px = px + 2 * dy1;
        else {
          if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
            y = y + 1;
          else
            y = y - 1;
          px = px + 2 * (dy1 - dx1);
        }
        this->pixel(x, y, color);
      }
    } else {
      if (dy >= 0) {
        x = x1;
        y = y1;
        ye = y2;
      } else {
        x = x2;
        y = y2;
        ye = y1;
      }

      this->pixel(x, y, color);

      for (i = 0; y < ye; i++) {
        y = y + 1;
        if (py <= 0)
          py = py + 2 * dx1;
        else {
          if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
            x = x + 1;
          else
            x = x - 1;
          py = py + 2 * (dx1 - dy1);
        }
        this->pixel(x, y, color);
      }
    }
    (void)i;
  }
  void addTriangle(vec3d p1, vec3d p2, vec3d p3, Uint32 color) {
    this->line(p1.x, p1.y, p2.x, p2.y, color);
    this->line(p2.x, p2.y, p3.x, p3.y, color);
    this->line(p3.x, p3.y, p1.x, p1.y, color);
  }
  void fillTriangle(vec3d p1, vec3d p2, vec3d p3, Uint32 color) {
    int x1 = p1.x;
    int x2 = p2.x;
    int x3 = p3.x;
    int y1 = p1.y;
    int y2 = p2.y;
    int y3 = p3.y;

    auto SWAP = [](int &x, int &y) {
      int t = x;
      x = y;
      y = t;
    };
    auto drawline = [&](int sx, int ex, int ny) {
      for (int i = sx; i <= ex; i++)
        this->pixel(i, ny, color);
    };

    int t1x, t2x, y, minx, maxx, t1xp, t2xp;
    bool changed1 = false;
    bool changed2 = false;
    int signx1, signx2, dx1, dy1, dx2, dy2;
    int e1, e2;
    // Sort vertices
    if (y1 > y2) {
      SWAP(y1, y2);
      SWAP(x1, x2);
    }
    if (y1 > y3) {
      SWAP(y1, y3);
      SWAP(x1, x3);
    }
    if (y2 > y3) {
      SWAP(y2, y3);
      SWAP(x2, x3);
    }

    t1x = t2x = x1;
    y = y1; // Starting points
    dx1 = (int)(x2 - x1);
    if (dx1 < 0) {
      dx1 = -dx1;
      signx1 = -1;
    } else
      signx1 = 1;
    dy1 = (int)(y2 - y1);

    dx2 = (int)(x3 - x1);
    if (dx2 < 0) {
      dx2 = -dx2;
      signx2 = -1;
    } else
      signx2 = 1;
    dy2 = (int)(y3 - y1);

    if (dy1 > dx1) { // swap values
      SWAP(dx1, dy1);
      changed1 = true;
    }
    if (dy2 > dx2) { // swap values
      SWAP(dy2, dx2);
      changed2 = true;
    }

    e2 = (int)(dx2 >> 1);
    // Flat top, just process the second half
    if (y1 == y2)
      goto next;
    e1 = (int)(dx1 >> 1);

    for (int i = 0; i < dx1;) {
      t1xp = 0;
      t2xp = 0;
      if (t1x < t2x) {
        minx = t1x;
        maxx = t2x;
      } else {
        minx = t2x;
        maxx = t1x;
      }
      // process first line until y value is about to change
      while (i < dx1) {
        i++;
        e1 += dy1;
        while (e1 >= dx1) {
          e1 -= dx1;
          if (changed1)
            t1xp = signx1; // t1x += signx1;
          else
            goto next1;
        }
        if (changed1)
          break;
        else
          t1x += signx1;
      }
      // Move line
    next1:
      // process second line until y value is about to change
      while (1) {
        e2 += dy2;
        while (e2 >= dx2) {
          e2 -= dx2;
          if (changed2)
            t2xp = signx2; // t2x += signx2;
          else
            goto next2;
        }
        if (changed2)
          break;
        else
          t2x += signx2;
      }
    next2:
      if (minx > t1x)
        minx = t1x;
      if (minx > t2x)
        minx = t2x;
      if (maxx < t1x)
        maxx = t1x;
      if (maxx < t2x)
        maxx = t2x;
      drawline(minx, maxx, y); // Draw line from min to max points found on the
                               // y Now increase y
      if (!changed1)
        t1x += signx1;
      t1x += t1xp;
      if (!changed2)
        t2x += signx2;
      t2x += t2xp;
      y += 1;
      if (y == y2)
        break;
    }
  next:
    // Second half
    dx1 = (int)(x3 - x2);
    if (dx1 < 0) {
      dx1 = -dx1;
      signx1 = -1;
    } else
      signx1 = 1;
    dy1 = (int)(y3 - y2);
    t1x = x2;

    if (dy1 > dx1) { // swap values
      SWAP(dy1, dx1);
      changed1 = true;
    } else
      changed1 = false;

    e1 = (int)(dx1 >> 1);

    for (int i = 0; i <= dx1; i++) {
      t1xp = 0;
      t2xp = 0;
      if (t1x < t2x) {
        minx = t1x;
        maxx = t2x;
      } else {
        minx = t2x;
        maxx = t1x;
      }
      // process first line until y value is about to change
      while (i < dx1) {
        e1 += dy1;
        while (e1 >= dx1) {
          e1 -= dx1;
          if (changed1) {
            t1xp = signx1;
            break;
          } // t1x += signx1;
          else
            goto next3;
        }
        if (changed1)
          break;
        else
          t1x += signx1;
        if (i < dx1)
          i++;
      }
    next3:
      // process second line until y value is about to change
      while (t2x != x3) {
        e2 += dy2;
        while (e2 >= dx2) {
          e2 -= dx2;
          if (changed2)
            t2xp = signx2;
          else
            goto next4;
        }
        if (changed2)
          break;
        else
          t2x += signx2;
      }
    next4:

      if (minx > t1x)
        minx = t1x;
      if (minx > t2x)
        minx = t2x;
      if (maxx < t1x)
        maxx = t1x;
      if (maxx < t2x)
        maxx = t2x;
      drawline(minx, maxx, y);
      if (!changed1)
        t1x += signx1;
      t1x += t1xp;
      if (!changed2)
        t2x += signx2;
      t2x += t2xp;
      y += 1;
      if (y > y3)
        return;
    }
  }

  void draw() {
    if (SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 255) < 0) {
      this->printSDLError();
      fail("Could not set the render draw color before clear");
    }
    if (SDL_RenderClear(this->renderer) < 0) {
      fail("Could not clear the renderer");
    }

    for (Pixel *p : this->pixels) {
      if (SDL_SetRenderDrawColor(this->renderer, p->col.r, p->col.g, p->col.b,
                                 p->col.a) < 0) {
        fail("Could not set render draw color before drawing point");
      }
      if (SDL_RenderDrawPointF(this->renderer, p->pos.x, p->pos.y) < 0) {
        fail("Could not render point");
      }
    }

    SDL_RenderPresent(this->renderer);
  }

  void poll(Keyboard *keyboard) {
    while (SDL_PollEvent(&this->event)) {
      if (this->event.type == SDL_QUIT) {
        SDL_Quit();
        exit(0);
      }
      if (this->event.type == SDL_KEYDOWN) {
        switch (this->event.key.keysym.sym) {
        case SDLK_UP:
          keyboard->ARROW_UP = true;
          break;
        case SDLK_DOWN:
          keyboard->ARROW_DOWN = true;
          break;
        case SDLK_LEFT:
          keyboard->ARROW_LEFT = true;
          break;
        case SDLK_RIGHT:
          keyboard->ARROW_RIGHT = true;
          break;
        case SDLK_w:
          keyboard->W = true;
          break;
        case SDLK_a:
          keyboard->A = true;
          break;
        case SDLK_s:
          keyboard->S = true;
          break;
        case SDLK_d:
          keyboard->D = true;
          break;
        }
      }
      if (this->event.type == SDL_KEYUP) {
        switch (this->event.key.keysym.sym) {
        case SDLK_UP:
          keyboard->ARROW_UP = false;
          break;
        case SDLK_DOWN:
          keyboard->ARROW_DOWN = false;
          break;
        case SDLK_LEFT:
          keyboard->ARROW_LEFT = false;
          break;
        case SDLK_RIGHT:
          keyboard->ARROW_RIGHT = false;
          break;
        case SDLK_w:
          keyboard->W = false;
          break;
        case SDLK_a:
          keyboard->A = false;
          break;
        case SDLK_s:
          keyboard->S = false;
          break;
        case SDLK_d:
          keyboard->D = false;
          break;
        }
      }
    }
  }

  void printSDLError() { std::cerr << SDL_GetError() << std::endl; }
};
