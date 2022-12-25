#pragma once

#include "direction.h"

namespace ghassanpl::geometry::squares
{
	template <typename T>
	concept metric = false;

	struct manhattan_metric
	{
		template <std::integral T>
		static constexpr auto distance(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			const auto d = glm::abs(b - a);
			return d.x + d.y;
		}

		template <std::integral T>
		static constexpr auto valid_neighbor(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			
		}

		static constexpr auto valid_direction(direction dir)
		{
			return is_cardinal(dir);
		}
	};
	using neighbor_metric = manhattan_metric;

	struct chebyshev_metric
	{
		template <std::integral T>
		static constexpr auto distance(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			const auto d = glm::abs(b - a);
			return max(d.x, d.y);
		}

		template <std::integral T>
		static constexpr bool is_valid_neighbor(glm::tvec2<T> a, glm::tvec2<T> b)
		{

		}

		static constexpr bool is_valid_direction(direction dir)
		{
			return is_valid(dir);
		}
	};
	using adjacent_metric = chebyshev_metric;

	template <std::integral T>
	constexpr auto manhattan_distance(glm::tvec2<T> a, glm::tvec2<T> b)
	{
		return manhattan_metric::distance(a, b);
	}

	template <std::integral T>
	constexpr auto chebyshev_distance(glm::tvec2<T> a, glm::tvec2<T> b)
	{
		return chebyshev_metric::distance(a, b);
	}

	inline constexpr bool is_surrounding(glm::ivec2 a, glm::ivec2 b) { return glm::abs(a.x - b.x) < 2 && glm::abs(a.y - b.y) < 2; }
	inline constexpr bool is_neighbor(glm::ivec2 a, glm::ivec2 b) { return is_surrounding(a, b) && glm::abs(a.y - b.y) != glm::abs(a.x - b.x); }
	inline constexpr bool is_diagonal_neighbor(glm::ivec2 a, glm::ivec2 b) { return is_surrounding(a, b) && glm::abs(a.y - b.y) == glm::abs(a.x - b.x); }

	inline constexpr glm::vec2 square_pos_to_world_pos(glm::ivec2 square_pos, glm::vec2 square_size) { return glm::vec2(square_pos) * square_size; }
	inline constexpr glm::vec2 square_pos_to_world_pos(glm::ivec2 square_pos, float square_size) { return glm::vec2(square_pos) * square_size; }
	inline constexpr rec2 world_rect_for_square(glm::ivec2 pos, glm::vec2 square_size) { return rec2::from_size(square_pos_to_world_pos(pos, square_size), square_size); }
	inline constexpr rec2 world_rect_for_square(glm::ivec2 pos, float square_size) { return rec2::from_size(square_pos_to_world_pos(pos, square_size), { square_size, square_size }); }
	inline constexpr glm::ivec2 world_pos_to_square_pos(glm::vec2 world_pos, glm::vec2 square_size) { return glm::ivec2(glm::floor(world_pos / square_size)); }
	inline constexpr glm::ivec2 world_pos_to_square_pos(glm::vec2 world_pos, float square_size) { return glm::ivec2(glm::floor(world_pos / square_size)); }
	inline constexpr irec2 world_rect_to_square_rect(rec2 const& world_rect, glm::vec2 square_size) { return irec2{ glm::floor(world_rect.p1 / square_size), glm::ceil(world_rect.p2 / square_size) }; }
	inline constexpr irec2 world_rect_to_square_rect(rec2 const& world_rect, float square_size) { return irec2{ glm::floor(world_rect.p1 / square_size), glm::ceil(world_rect.p2 / square_size) }; }

	template <metric METRIC = chebyshev_metric>
	struct square_space
	{
		glm::vec2 square_size;

		constexpr glm::vec2 to_world_pos(glm::ivec2 square_pos) const noexcept { return square_pos_to_world_pos(square_pos, square_size); }
		constexpr rec2 world_rect_for_square(glm::ivec2 square_pos) const noexcept { return ghassanpl::geometry::squares::world_rect_for_square(square_pos, square_size); }
		constexpr glm::ivec2 to_square_pos(glm::vec2 world_pos) const noexcept { return world_pos_to_square_pos(world_pos, square_size); }
		constexpr irec2 to_square_rect(rec2 const& world_rect) const noexcept { return world_rect_to_square_rect(world_rect, square_size); }
	};

	using square_pos = named<glm::ivec2, "square_pos", traits::location>;
	using world_pos = named<glm::vec2, "world_pos", traits::location>;

	using square_rec = named<irec2, "square_rec">;
	using world_rec = named<rec2, "world_rec">;
}