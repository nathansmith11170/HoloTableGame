module;
#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>
export module game:renderer;

namespace glsl_math
{
struct vec2
{
	float x;
	float y;
};

struct vec3
{
	float x;
	float y;
	float z;
};

struct vec4
{
	float x;
	float y;
	float z;
	float w;

	vec4(vec2 v, float z, float w) : x{v.x}, y{v.y}, z{z}, w{w} {}
	vec4(vec3 v, float w) : x{v.x}, y{v.y}, z{v.z}, w{w} {}
};

template <class>
struct vec_traits;

template <>
struct vec_traits<vec2>
{
	using value_type = float;
	static constexpr std::size_t size = 2;
	static auto data(vec2& v) noexcept -> value_type* { return &v.x; }
	static auto data(const vec2& v) noexcept -> const value_type* { return &v.x; }
};

template <>
struct vec_traits<vec3>
{
	using value_type = float;
	static constexpr std::size_t size = 3;
	static auto data(vec3& v) noexcept -> value_type* { return &v.x; }
	static auto data(const vec3& v) noexcept -> const value_type* { return &v.x; }
};

template <>
struct vec_traits<vec4>
{
	using value_type = float;
	static constexpr std::size_t size = 4;
	static auto data(vec4& v) noexcept -> value_type* { return &v.x; }
	static auto data(const vec4& v) noexcept -> const value_type* { return &v.x; }
};

template <class V>
concept vec_like = requires(V v) {
	typename vec_traits<V>::value_type;
	{ vec_traits<V>::size } -> std::convertible_to<std::size_t>;
	{ vec_traits<V>::data(v) } -> std::same_as<typename vec_traits<V>::value_type*>;
	{ vec_traits<V>::data(std::as_const(v)) } -> std::same_as<const typename vec_traits<V>::value_type*>;
};

template <vec_like V>
constexpr auto size_v = vec_traits<V>::size;

template <vec_like V>
using value_t = typename vec_traits<V>::value_type;

template <vec_like V>
auto data_ptr(V& v) noexcept -> value_t<V>* { return vec_traits<V>::data(v); }

template <vec_like V>
auto data_ptr(const V& v) noexcept -> const value_t<V>* { return vec_traits<V>::data(v); }

template <vec_like LHS, vec_like RHS>
	requires(size_v<LHS> == size_v<RHS> &&
					 std::same_as<value_t<LHS>, value_t<RHS>>)
auto operator-(const LHS& a, const RHS& b)
{
	LHS out{};
	auto* dst = data_ptr(out);
	auto* lhs = data_ptr(a);
	auto* rhs = data_ptr(b);
	for (std::size_t i = 0; i < size_v<LHS>; ++i)
		dst[i] = lhs[i] - rhs[i];
	return out;
}

template <vec_like V>
auto length(const V& v) -> float
{
	float scale = 0.0f;
	float sum_sq = 1.0f;

	for (std::size_t i = 0; i < size_v<V>; ++i)
	{
		float x = data_ptr(v)[i];
		float ax = std::fabs(x);

		if (ax != 0.0f)
		{
			if (scale < ax)
			{
				float t = scale / ax; // old scale relative to new largest
				sum_sq = sum_sq * t * t + 1.0f;
				scale = ax;
			}
			else
			{
				float t = ax / scale;
				sum_sq += t * t;
			}
		}
	}

	return scale == 0.0f ? 0.0f : scale * std::sqrt(sum_sq);
}

template <std::floating_point T>
auto smoothstep(T edge0, T edge1, T x) -> T
{
	if (edge0 == edge1)
		return x < edge0 ? T{0} : T{1};

	T t = std::clamp((x - edge0) / (edge1 - edge0), T{0}, T{1});
	return t * t * (T{3} - T{2} * t);
}

template <vec_like V>
auto smoothstep(const V& edge0, const V& edge1, const V& x) -> V
{
	V out{};
	auto* dst = data_ptr(out);
	auto* e0 = data_ptr(edge0);
	auto* e1 = data_ptr(edge1);
	auto* xv = data_ptr(x);
	for (std::size_t i = 0; i < size_v<V>; ++i)
		dst[i] = smoothstep(e0[i], e1[i], xv[i]);
	return out;
}

template <vec_like V, std::floating_point T>
	requires std::convertible_to<T, value_t<V>>
auto smoothstep(T edge0, T edge1, const V& x) -> V
{
	V out{};
	auto* dst = data_ptr(out);
	auto* xv = data_ptr(x);
	for (std::size_t i = 0; i < size_v<V>; ++i)
		dst[i] = static_cast<value_t<V>>(smoothstep(edge0, edge1, xv[i]));
	return out;
}
} // namespace glsl_math

export namespace game::rendering
{
auto circle_vertex_shader(::glsl_math::vec2 position) -> ::glsl_math::vec4
{
	// For GLSL this would be setting gl_Position
	return {position, 0.0f, 1.0f};
}

auto circle_fragment_shader(::glsl_math::vec2 pixel_coord,
														::glsl_math::vec2 circle_center,
														float circle_radius,
														::glsl_math::vec3 circle_color) -> ::glsl_math::vec4
{
	::glsl_math::vec2 pos = pixel_coord - circle_center;

	float dist = ::glsl_math::length(pos);
	float sdf = dist - circle_radius;

	float alpha = 1.0f - ::glsl_math::smoothstep(-1.0f, 1.0f, sdf);

	// For GLSL this would be setting a global
	return {circle_color, alpha};
}
} // namespace game::rendering
