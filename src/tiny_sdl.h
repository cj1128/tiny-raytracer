#ifndef TINY_SDL_H
#define TINY_SDL_H

#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct img_texture {
  SDL_Texture *texture;
  int w;
  int h;
};

struct font_texture {
  SDL_Texture *texture;
  int w;
  int h;
};

void
InitSDL()
{
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    printf("error: %s\n", SDL_GetError());
    exit(1);
  }
}

SDL_Window *
InitWindow(int width, int height, char *name = "My Window")
{
  SDL_Window *win = SDL_CreateWindow(name,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    width,
    height,
    SDL_WINDOW_SHOWN);

  if(win == NULL) {
    printf("error: %s\n", SDL_GetError());
    exit(1);
  }

  return win;
}

SDL_Renderer *
InitRenderer(SDL_Window *win, bool vsync = false)
{
  uint32_t flags = SDL_RENDERER_ACCELERATED;
  if(vsync) {
    flags |= SDL_RENDERER_PRESENTVSYNC;
  }
  SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, flags);
  if(renderer == NULL) {
    printf("error: %s\n", SDL_GetError());
    exit(1);
  }

  return renderer;
}

#endif
