#include <stdio.h>
#include "tiny_platform.h"
#include "tiny_math.h"
#include "tiny_sdl.h"

#define WIDTH 800
#define HEIGHT 800

#define INF 999999.0f

struct s_sphere {
  v3 center;
  f32 radius;
  v3 color;
  int specular; // 0 means turn off
};

enum light_type {
  LightType_Ambient,
  LightType_Point,
  LightType_Directional,
};

struct s_light {
  light_type type;
  f32 intensity;
  v3 position; // used for point
  v3 direction; // used for directional
};

f32
ComputeLighting(v3 point,
  v3 normal,
  v3 v,
  s_light *lights,
  int lightCount,
  int specular)
{
  f32 result = 0.0f;
  for(int lightIndex = 0; lightIndex < lightCount; lightIndex++) {
    s_light *light = lights + lightIndex;

    if(light->type == LightType_Ambient) {
      result += light->intensity;
    } else {
      v3 l = light->type == LightType_Point ? light->position - point
                                            : light->direction;

      // diffuse
      f32 nDotL = Inner(normal, l);
      if(nDotL > 0) {
        result += (light->intensity * nDotL) / (Length(normal) * Length(l));
      }

      // specular
      if(specular != 0) {
        v3 r = 2 * normal * nDotL - l;
        f32 rDotV = Inner(r, v);
        if(rDotV > 0) {
          result += light->intensity
            * Pow(rDotV / (Length(r) * Length(v)), (f32)specular);
        }
      }
    }
  }

  return result;
}

v3
CanvasToViewport(v2 viewSize,
  int x,
  int y,
  int canvsWidth,
  int canvasHeight,
  f32 projectionD)
{
  v3 result = {};
  result.x = ((f32)x / (f32)canvsWidth) * viewSize.width;
  result.y = ((f32)y / (f32)canvasHeight) * viewSize.height;
  result.z = projectionD;

  return result;
}

v2
IntersectRaySphere(v3 origin, v3 point, s_sphere *s)
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
RayTrace(s_sphere *spheres, int count, v3 origin, v3 vp, f32 tMin, f32 tMax)
{
  f32 closestT = INF;
  s_sphere *closestSphere = NULL;

  s_light lights[] = {
    {
      .type = LightType_Ambient,
      .intensity = 0.2f,
    },
    {
      .type = LightType_Point,
      .intensity = 0.6f,
      .position = V3(2, 1, 0),
    },
    {
      .type = LightType_Directional,
      .intensity = 0.2f,
      .direction = V3(1, 4, 4),
    },
  };

  v3 backgroundColor = { 1.0f, 1.0f, 1.0f };

  for(int i = 0; i < count; i++) {
    s_sphere *s = spheres + i;
    v2 intersectResult = IntersectRaySphere(origin, vp, s);

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

  // printf("%f.%f.%f\n", vp.x, vp.y, vp.z);

  // basic
  // return closestSphere->color;

  v3 d = vp - origin;
  v3 point = origin + closestT * d;
  v3 n = point - closestSphere->center;
  v3 v = origin - point;
  f32 invNLength = 1.0f / Length(n);
  n *= invNLength;

  v3 color = Clamp01(closestSphere->color
    * ComputeLighting(point,
      n,
      v,
      lights,
      ArrayCount(lights),
      closestSphere->specular));

  return color;
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

// NOTE: y is top-down
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
RenderRayTracking(void *memory, int pitch, int canvasWidth, int canvasHeight)
{
  v3 origin = { 0.0f, 0.0f, 0.0f };

  v2 viewSize = { 1.0f, 1.0f };
  f32 projectionD = 1.0f;

  s_sphere spheres[4] = {
    {
      .center = V3(0, -1, 3),
      .radius = 1.0f,
      .color = V3(1, 0, 0), // red
      .specular = 500, // shiny
    },
    {
      .center = V3(2, 0, 4),
      .radius = 1.0f,
      .color = V3(0, 0, 1), // blue
      .specular = 500, // shiny
    },
    {
      .center = V3(-2, 0, 4),
      .radius = 1.0f,
      .color = V3(0, 1, 0), // green
      .specular = 10, // somewhat shiny
    },
    {
      .center = V3(0, -5001, 0),
      .radius = 5000.0f,
      .color = V3(1, 1, 0), // yellow
      .specular = 1000, // very shiny
    },
  };

  Assert(canvasWidth % 2 == 0);
  Assert(canvasHeight % 2 == 0);

  int halfWidth = canvasWidth / 2;
  int halfHeight = canvasHeight / 2;

  for(int x = -halfWidth; x < halfWidth; x++) {
    for(int y = -halfHeight; y < halfHeight; y++) {
      v3 vp = CanvasToViewport(viewSize,
        x,
        y,
        canvasWidth,
        canvasHeight,
        projectionD);

      v3 color = RayTrace(spheres, ArrayCount(spheres), origin, vp, 1.0f, INF);

      PutPixel(memory, pitch, canvasWidth, canvasHeight, x, y, color);
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

  RenderRayTracking(rawPixels, pitch, WIDTH, HEIGHT);
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
