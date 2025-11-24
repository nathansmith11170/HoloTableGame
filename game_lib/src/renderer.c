#include <math.h>
#include <renderer.h>
#include <stdbool.h>
#include <stddef.h>

float vec2_length(vec2 v)
{
  return sqrtf(v.x * v.x + v.y * v.y);
}

float smoothstep(float edge0, float edge1, float x)
{
  if (edge0 == edge1)
  {
    return x < edge0 ? 0.0f : 1.0f;
  }
  float t = (x - edge0) / (edge1 - edge0);
  t = t < 0.0f ? 0.0f : t;
  t = t > 1.0f ? 1.0f : t;
  return t * t * (3.0f - 2.0f * t);
}

vec4 circle_vertex_shader(vec2 position)
{
  vec4 result = {position.x, position.y, 0.0f, 1.0f};
  return result;
}

vec4 circle_fragment_shader(vec2 pixel_coord, vec2 circle_center, float circle_radius, vec3 circle_color)
{
  vec2 pos = {pixel_coord.x - circle_center.x, pixel_coord.y - circle_center.y};

  float dist = vec2_length(pos);
  float sdf = dist - circle_radius;

  float alpha = 1.0f - smoothstep(-1.0f, 1.0f, sdf);

  vec4 result = {circle_color.x, circle_color.y, circle_color.z, alpha};
  return result;
}
