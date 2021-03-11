#include <stdio.h>
#include "tiny_platform.h"
#include "tiny_math.h"
#include "tiny_sdl.h"

#define WIDTH 800
#define HEIGHT 800

#define INF 999999.0f

struct sphere {
  v3 center;
  f32 radius;
  v3 color;
};

inline v3
CanvasToViewport(v2 vp,
  int x,
  int y,
  int canvsWidth,
  int canvasHeight,
  f32 projectionD)
{
  v3 result = {};
  result.x = ((f32)x / (f32)canvsWidth) * vp.width;
  result.y = ((f32)y / (f32)canvasHeight) * vp.height;
  result.z = projectionD;

  return result;
}

v2
IntersectRaySphere(v3 origin, v3 point, sphere *s)
{
  v3 d = point - origin;
  v3 co = origin - s->center;

  f32 a = Inner(d, d);
  f32 b = 2 * Inner(co, d);
  f32 c = Inner(co, co) - s->radius * s->radius;

  f32 discriminant = Square(b) - 4 * a * c;

  if(discriminant < 0) {
    return V2(INF, INF);
  }

  v2 result = {};
  result.e[0] = (-b + SquareRoot(discriminant)) / (2 * a);
  result.e[1] = (-b - SquareRoot(discriminant)) / (2 * a);

  return result;
}

v3
RayTrace(sphere *spheres, int count, v3 origin, v3 point, f32 tMin, f32 tMax)
{
  f32 closestT = INF;
  sphere *closestSphere = NULL;

  v3 backgroundColor = { 1.0f, 1.0f, 1.0f };

  for(int i = 0; i < count; i++) {
    sphere *s = spheres + i;
    v2 intersectResult = IntersectRaySphere(origin, point, s);

    f32 v = intersectResult.e[0];

    if((v > tMin && v < tMax) && v < closestT) {
      closestT = v;
      closestSphere = s;
    }

    v = intersectResult.e[1];
    if((v > tMin && v < tMax) && v < closestT) {
      closestT = v;
      closestSphere = s;
    }
  }

  if(closestSphere == NULL) {
    return backgroundColor;
  }

  // printf("%f.%f.%f\n", point.x, point.y, point.z);

  return closestSphere->color;
}

inline u32
ColorToRGBA(v3 color)
{
  u32 r = FloorReal32ToUint32(color.r * 255.0f + 0.5f);
  u32 g = FloorReal32ToUint32(color.g * 255.0f + 0.5f);
  u32 b = FloorReal32ToUint32(color.b * 255.0f + 0.5f);
  u32 a = 255;
  u32 result = (u32)((r << 24) | (g << 16) | (b << 8) | (a << 0));
  return result;
}

void
PutPixel(void *memory, int pitch, int width, int height, int x, int y, v3 color)
{
  int fx = x + width / 2;
  int fy = -y + height / 2 - 1;

  Assert(fx >= 0 && fx < width);
  Assert(fy >= 0 && fy < height);

  typedef uint32_t(*pixels_t)[pitch / 4];
  pixels_t pixels = (pixels_t)memory;
  pixels[fy][fx] = ColorToRGBA(color);
}

void
RenderTestGradient(void *memory, int pitch, int width, int height)
{
  typedef uint32_t(*pixels_t)[pitch / 4];
  pixels_t pixels = (pixels_t)memory;

  for(int y = 0; y < height; y++)
    for(int x = 0; x < width; x++) {
      uint8_t r = 255;
      uint8_t g = (uint8_t)x;
      uint8_t b = (uint8_t)y;
      pixels[y][x] = (uint32_t)((r << 24) | (g << 16) | (b << 8) | 0xff);
    }
}

void
RenderRayTracing(void *memory, int pitch, int width, int height)
{
  v3 origin = { 0.0f, 0.0f, 0.0f };

  v2 vp = { 1.0f, 1.0f };
  f32 projectionD = 1.0f;

  sphere spheres[3];

  spheres[0].center = V3(0, -1, 3);
  spheres[0].radius = 1;
  spheres[0].color = V3(1.0f, 0.0f, 0.0f); // red

  spheres[1].center = V3(2, 0, 4);
  spheres[1].radius = 1;
  spheres[1].color = V3(0.0f, 0.0f, 1.0f); // blue

  spheres[2].center = V3(-2, 0, 4);
  spheres[2].radius = 1;
  spheres[2].color = V3(0.0f, 1.0f, 0.0f); // green

  Assert(width % 2 == 0);
  Assert(height % 2 == 0);

  int halfWidth = width / 2;
  int halfHeight = height / 2;

  for(int x = -halfWidth; x < halfWidth; x++) {
    for(int y = -halfHeight; y < halfHeight; y++) {
      v3 point = CanvasToViewport(vp, x, y, width, height, projectionD);

      v3 color
        = RayTrace(spheres, ArrayCount(spheres), origin, point, 1.0f, INF);

      PutPixel(memory, pitch, width, height, x, y, color);
    }
  }
}

int
main()
{
  InitSDL();

  SDL_Window *win = InitWindow(WIDTH, HEIGHT, "Tiny RayTracer");

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

  // NOTE: y is top-down
  RenderRayTracing(rawPixels, pitch, WIDTH, HEIGHT);
  // RenderTestGradient(rawPixels, pitch, WIDTH, HEIGHT);

  SDL_UnlockTexture(texture);

  bool quit = false;

  SDL_Event e;

  while(SDL_PollEvent(&e)) {}

  SDL_Rect rect;

  // Clear
  SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
  SDL_RenderClear(renderer);

  // Texture
  SDL_RenderCopy(renderer, texture, NULL, NULL);

  SDL_RenderPresent(renderer);

  while(!quit) {
    SDL_WaitEvent(&e);

    if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
      quit = true;
    }

    if(e.type == SDL_QUIT) {
      quit = true;
    }
  }

  return 0;
}
