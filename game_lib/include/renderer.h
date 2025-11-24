#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H
typedef struct
{
  float x;
  float y;
} vec2;

typedef struct
{
  float x;
  float y;
  float z;
} vec3;

typedef struct
{
  float x;
  float y;
  float z;
  float w;
} vec4;

vec4 circle_vertex_shader(vec2 position);
vec4 circle_fragment_shader(vec2 pixel_coord, vec2 circle_center, float circle_radius, vec3 circle_color);
#endif // GAME_RENDERER_H
