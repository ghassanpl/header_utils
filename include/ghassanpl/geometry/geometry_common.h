/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "../enum_flags.h"
#include "../align.h"
#include "../named.h"
#include "../rec2.h"
#include "../constexpr_math.h"

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <format>

namespace glm
{
	template <typename STRINGIFIER>
	bool stringify(STRINGIFIER& str, glm::vec4& b) { return str('[', b.x, ',', b.y, ',', b.z, ',', b.w, ']'); }
	template <typename STRINGIFIER>
	bool stringify(STRINGIFIER& str, glm::vec4 const& b) { return str('[', b.x, ',', b.y, ',', b.z, ',', b.w, ']'); }

	template <typename STRINGIFIER>
	bool stringify(STRINGIFIER& str, glm::vec2& b) { return str('[', b.x, ',', b.y, ']'); }
	template <typename STRINGIFIER>
	bool stringify(STRINGIFIER& str, glm::vec2 const& b) { return str('[', b.x, ',', b.y, ']'); }
}

namespace ghassanpl::geometry
{
	template <std::floating_point T> using basic_radians_t = named<T, "radians", traits::displacement>;
	template <std::floating_point T> using basic_degrees_t = named<T, "degrees", traits::displacement>;
	
	template <std::floating_point T> using basic_heading_t = named<T, "heading", traits::location, traits::is_location_of<basic_degrees_t<T>>>;

	using degrees = basic_degrees_t<float>;
	using radians = basic_radians_t<float>;
	using heading = basic_heading_t<float>;

	template <std::floating_point T>
	inline constexpr basic_degrees_t<T> ensure_positive(basic_degrees_t<T> degrees) noexcept { return basic_degrees_t<T>{ cem::fmod(degrees.value, T(360)) }; }
	template <std::floating_point T>
	inline constexpr basic_radians_t<T> ensure_positive(basic_radians_t<T> degrees) noexcept { return basic_radians_t<T>{ cem::fmod(degrees.value, glm::radians(T(360))) }; }

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


	inline constexpr auto full_circle = degrees{ 360.0f };
	inline constexpr auto half_circle = degrees{ 360.0f / 2.0f };
	inline constexpr auto quarter_circle = degrees{ 360.0f / 4.0f };

	template <size_t NTH_SLICE, size_t SLICE_COUNT, degrees starting_at = degrees{ 0.0f } >
	inline constexpr std::pair<degrees, degrees> circle_slice = {
		ensure_positive(degrees{NTH_SLICE * (360.0f / SLICE_COUNT)} + starting_at),
		ensure_positive(degrees{(NTH_SLICE + 1) * (360.0f / SLICE_COUNT)} + starting_at)
	};

	using rec2 = trec2<float>;
	using irec2 = trec2<int>;
}

template <typename T>
struct std::formatter<glm::tvec2<T>> : std::formatter<std::string>
{
	template<class FormatContext>
	auto format(glm::tvec2<T> const& p, FormatContext& ctx) { return std::formatter<string>::format(std::format("[{}, {}]", p.x, p.y), ctx); }
};

template <typename T>
struct std::formatter<glm::tvec3<T>> : std::formatter<std::string>
{
	template<class FormatContext>
	auto format(glm::tvec2<T> const& p, FormatContext& ctx) { return std::formatter<string>::format(std::format("[{}, {}, {}]", p.x, p.y, p.z), ctx); }
};

template <typename T>
struct std::formatter<glm::tvec4<T>> : std::formatter<std::string>
{
	template<class FormatContext>
	auto format(glm::tvec2<T> const& p, FormatContext& ctx) { return std::formatter<string>::format(std::format("[{}, {}, {}, {}]", p.x, p.y, p.z, p.w), ctx); }
};
