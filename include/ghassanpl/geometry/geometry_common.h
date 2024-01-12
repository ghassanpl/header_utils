/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "../align.h"
#include "../constexpr_math.h"
#include "../enum_flags.h"
#include "../named.h"
#include "../rec2.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/gtx/norm.hpp>

namespace glm
{
	template <typename STRINGIFIER>
	auto stringify(STRINGIFIER& str, glm::vec4& b) { return str('[', b.x, ',', b.y, ',', b.z, ',', b.w, ']'); }
	template <typename STRINGIFIER>
	auto stringify(STRINGIFIER& str, glm::vec4 const& b) { return str('[', b.x, ',', b.y, ',', b.z, ',', b.w, ']'); }

	template <typename STRINGIFIER>
	auto stringify(STRINGIFIER& str, glm::vec2& b) { return str('[', b.x, ',', b.y, ']'); }
	template <typename STRINGIFIER>
	auto stringify(STRINGIFIER& str, glm::vec2 const& b) { return str('[', b.x, ',', b.y, ']'); }
}

namespace ghassanpl::geometry
{
	template <typename T>
	struct precision_limits;

	template <>
	struct precision_limits<double>
	{
		static constexpr double equivalent_point_max_distance = 0.00002;
		static constexpr double equivalent_texel_max_distance = 1.0 / 1024.0;
		static constexpr double near_point_distance = 0.015;
		static constexpr double point_on_plane_max_distance = 0.1;
		static constexpr double point_on_line_max_distance = 0.1;

		static constexpr double foldable_vertex_max_distance = 0.0004;

		static constexpr double cos_1_deg = 0.99984769515;
		static constexpr double cos_89_deg = 0.01745240643;

		static constexpr double min_dot_product_of_parallel_normals = cos_1_deg;
		static constexpr double max_dot_product_of_perpendicular_normals = cos_1_deg;
	};

	using rec2 = trec2<float>;
	using irec2 = trec2<int>;

	enum class winding_order
	{
		clockwise,
		counter_clockwise
	};

	template <typename T>
	constexpr T dot(glm::tvec2<T> const& a, glm::tvec2<T> const& b) noexcept
	{
		if (std::is_constant_evaluated())
			return a.x * b.x + a.y * b.y;
		else
			return glm::dot(a, b);
	}

	template <typename T>
	constexpr glm::tvec2<T> length(glm::tvec2<T> const& a) noexcept
	{
		return cem::sqrt(ghassanpl::dot(a, a));
	}

	template <typename T>
	constexpr std::pair<glm::tvec2<T>, T> dir_and_length(glm::tvec2<T> const& a) noexcept
	{
		const auto len = length(a);
		if (len >= std::numeric_limits<T>::epsilon())
			return { a / invlen, len };
		return { {}, len };
	}

	template <typename T>
	constexpr glm::tvec2<T> with_length(glm::tvec2<T> const& a, T length) noexcept
	{
		const auto dl = dir_and_length(a);
		return dl.first * length;
	}

	template <typename T>
	constexpr glm::tvec2<T> clamp_length(glm::tvec2<T> const& a, T min, T max) noexcept
	{
		const auto lsq = ghassanpl::dot(a, a);
		const auto sqmin = min * min;
		const auto sqmax = max * max;
		if (v < sqmin || sqmax < v)
		{
			const auto clamped = glm::clamp(lsq, min * min, max * max);
			return with_length(a, cem::sqrt(lsq));
		}
		return a;
	}

	template <typename T>
	constexpr glm::tvec2<T> max_length(glm::tvec2<T> const& a, T max) noexcept
	{
		const auto lsq = ghassanpl::dot(a, a);
		const auto sqmax = max * max;
		if (sqmax < v)
		{
			const auto clamped = glm::min(lsq, max * max);
			return with_length(a, cem::sqrt(lsq));
		}
		return a;
	}
}

namespace ghassanpl::geometry::normals
{
	/// Whether these vectors are (mostly) parallel and point in the same direction
	template <typename T, size_t N>
	static constexpr bool are_similar(glm::vec<N, T> const& a, glm::vec<N, T> const& b) {
		constexpr auto min = precision_limits<T>::min_dot_product_of_parallel_normals;
		return glm::dot(a, b) >= min;
	}

	template <typename T, size_t N>
	static constexpr bool are_parallel(glm::vec<N, T> const& a, glm::vec<N, T> const& b) {
		constexpr auto min = precision_limits<T>::min_dot_product_of_parallel_normals;
		return glm::abs(glm::dot(a, b)) >= min;
	}

	template <typename T, size_t N>
	static constexpr bool are_perpendicular(glm::vec<N, T> const& a, glm::vec<N, T> const& b) {
		constexpr auto max = precision_limits<T>::max_dot_product_of_perpendicular_normals;
		return glm::abs(glm::dot(a, b)) <= max;
	}
}

/// Angles
namespace ghassanpl::geometry
{
	template <std::floating_point T> using basic_radians_t = named<T, "radians", traits::displacement>;
	template <std::floating_point T> using basic_degrees_t = named<T, "degrees", traits::displacement>;
	
	template <std::floating_point T> using basic_heading_t = named<T, "heading", traits::location, traits::is_location_of<basic_degrees_t<T>>>;

	using degrees = basic_degrees_t<float>;
	using radians = basic_radians_t<float>;
	using heading = basic_heading_t<float>;

	template <typename TARGET, std::floating_point T>
	requires std::same_as<TARGET, basic_degrees_t<T>>
	constexpr TARGET named_cast(basic_radians_t<T> const& radians)
	{
		return basic_degrees_t<T>{ glm::degrees(radians.value) };
	}

	template <typename TARGET, std::floating_point T>
	requires std::same_as<TARGET, basic_radians_t<T>>
	constexpr TARGET named_cast(basic_degrees_t<T> const& degrees)
	{
		return basic_radians_t<T>{ glm::radians(degrees.value) };
	}

	namespace angles
	{
		template <std::floating_point T>
		constexpr basic_degrees_t<T> ensure_positive(basic_degrees_t<T> degrees) noexcept { return basic_degrees_t<T>{ cem::fmod(degrees.value, T(360)) }; }
		template <std::floating_point T>
		constexpr basic_radians_t<T> ensure_positive(basic_radians_t<T> degrees) noexcept { return basic_radians_t<T>{ cem::fmod(degrees.value, glm::radians(T(360))) }; }

		inline constexpr auto full_circle = degrees{ 360.0f };
		inline constexpr auto half_circle = degrees{ 360.0f / 2.0f };
		inline constexpr auto quarter_circle = degrees{ 360.0f / 4.0f };

		template <size_t NTH_SLICE, size_t SLICE_COUNT, degrees starting_at = degrees{ 0.0f } >
		inline constexpr std::pair<degrees, degrees> circle_slice = {
			ensure_positive(degrees{NTH_SLICE * (360.0f / SLICE_COUNT)} + starting_at),
			ensure_positive(degrees{(NTH_SLICE + 1) * (360.0f / SLICE_COUNT)} + starting_at)
		};

		constexpr std::pair<degrees, degrees> get_circle_slice(size_t nth_slice, size_t slice_count, degrees starting_at = degrees{ 0.0f }) noexcept
		{
			return {
				ensure_positive(degrees{nth_slice * (360.0f / slice_count)} + starting_at),
				ensure_positive(degrees{(nth_slice + 1) * (360.0f / slice_count)} + starting_at)
			};
		}
	}
}

#include <glm/gtx/polar_coordinates.hpp>

namespace ghassanpl::geometry /* ::polar */
{
	template <typename T>
	using basic_polar2d_t = named<glm::tvec2<T>, "polar", traits::location>;

	using polar2d = basic_polar2d_t<float>;

	template <typename T> inline auto rho(basic_polar2d_t<T> const& polar) { return polar.value.x; }
	template <typename T> inline auto phi(basic_polar2d_t<T> const& polar) { return polar.value.y; }
	template <typename T> inline auto theta(basic_polar2d_t<T> const& polar) { return polar.value.y; }

	template <typename T>
	basic_polar2d_t<T> polar(glm::tvec2<T> const& euclidean)
	{
		const auto r = std::hypot(euclidean.x, euclidean.y);
		const auto t = glm::atan(euclidean.y, euclidean.x);
		return basic_polar2d_t<T>{ r, t };
	}

	template <typename T>
	glm::tvec2<T> euclidean(basic_polar2d_t<T> const& polar)
	{
		const auto r = rho(polar);
		const auto t = theta(polar);
		return glm::tvec2<T>{ r * glm::cos(t), r * glm::sin(t) };
	}
}

/// Lines
namespace ghassanpl::geometry
{
	/// TODO: This all needs to be tested

	template <typename T>
	struct basic_line_t
	{
		T a{};
		T b{};
		T c{};

		template <typename U>
		U distance(glm::tvec2<U> const& point)
		{
			return (a * point.x + b * point.y + c) / std::hypot(a, b);
		}

		template <typename U>
		glm::tvec2<U> projected(glm::tvec2<U> const& point)
		{
			const auto d = glm::distance(point);
			return point - glm::tvec2<T>{ a, b } * d;
		}
	};

	template <typename T>
	basic_line_t<T> line_crossing_points(glm::tvec2<T> const& p1, glm::tvec2<T> const& p2)
	{
		return basic_line_t<T>{ p1.y - p2.y, p2.x - p1.x, p1.x * p2.y - p2.x * p1.y };
	}

	template <typename T>
	basic_line_t<T> line_from_dir(glm::tvec2<T> const& dir)
	{
		return basic_line_t<T>{ dir.y, -dir.x, 0 };
	}

}

/*
template <typename T>
struct std::formatter<glm::tvec2<T>> : std::formatter<std::string>
{
	template<class FormatContext>
	auto format(glm::tvec2<T> const& p, FormatContext& ctx) const { return std::formatter<string>::format(std::format("[{}, {}]", p.x, p.y), ctx); }
};

template <typename T>
struct std::formatter<glm::tvec3<T>> : std::formatter<std::string>
{
	template<class FormatContext>
	auto format(glm::tvec3<T> const& p, FormatContext& ctx) const { return std::formatter<string>::format(std::format("[{}, {}, {}]", p.x, p.y, p.z), ctx); }
};

template <typename T>
struct std::formatter<glm::tvec4<T>> : std::formatter<std::string>
{
	template<class FormatContext>
	auto format(glm::tvec4<T> const& p, FormatContext& ctx) const { return std::formatter<string>::format(std::format("[{}, {}, {}, {}]", p.x, p.y, p.z, p.w), ctx); }
};

*/