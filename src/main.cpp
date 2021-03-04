#include <stdio.h>
#include "tiny_platform.h"
#include "tiny_math.h"
#include "tiny_sdl.h"

#define WIDTH 800
#define HEIGHT 600

int
main()
{
  InitSDL();

  SDL_Window *win = InitWindow(WIDTH, HEIGHT);

  SDL_Renderer *renderer = InitRenderer(win);

  SDL_Texture *texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_STREAMING,
    WIDTH,
    HEIGHT);
  if(texture == NULL) {
    return 1;
  }

  void *rawPixels;
  int pitch;
  SDL_LockTexture(texture, NULL, &rawPixels, &pitch);

  int width = pitch / 4;
  typedef uint32_t(*pixels_t)[width];
  pixels_t pixels = (pixels_t)rawPixels;

  for(int y = 0; y < HEIGHT; y++)
    for(int x = 0; x < WIDTH; x++) {
      uint8_t r = 0;
      uint8_t g = (uint8_t)x;
      uint8_t b = (uint8_t)y;
      pixels[y][x] = (uint32_t)((r << 24) | (g << 16) | (b << 8) | 0xff);
    }

  SDL_UnlockTexture(texture);

  SDL_Event e;
  bool quit = false;
  while(!quit) {
    while(SDL_PollEvent(&e)) {
      if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        quit = true;
      }

      if(e.type == SDL_QUIT) {
        quit = true;
      }
    }

    SDL_Rect rect;

    // Clear
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    // Texture
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    SDL_RenderPresent(renderer);
  }

  return 0;
}
