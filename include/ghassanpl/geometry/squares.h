/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "direction.h"

namespace ghassanpl::geometry::squares
{
	/// \defgroup Squares Squares
	/// \ingroup Geometry
	/// Operations on positions in a 2D square grid.
	/// 
	/// Square positions are defined as two integers `x` and `y`, in the primary position type `glm::ivec2`.
	/// The distances between these positions are dependent on the metric chosen.
	/// 
	/// Definitions:
	/// * `tile` - another name for square
	/// * `metric` - the means to calculate distances between two squares
	/// * `adjacency` - in a specified metric, two squares are considered "adjacent" if the distance between their positions is 1
	/// * `neighbor of S` - a square N is considered the neighbor of square S if it shares an edge with S
	/// * `diagonal neighbor of S` - a square N is considered the diagonal neighbor of square S if it shares a vertex with S
	/// * `square surrounding S` - a square N is considered in the surrounding of square S if it shares an edge OR vertex with S
	/// * `the surrounding of S` - all squares that are considered to be `surrounding S`
	/// * `tile space` - a combination of a metric and a tile size; usually defined as part of a world
	/// * `world` - a set of unique squares that form a grid within a tile space, either unbounded, or bounded by a rectangle edge; the squares have integral positions,
	///		and every position within the bounds refers to a unique square, but world can have non-integral "world" positions defined on it as well
	/// * `tile position` - the integral position of a single tile in a world; every position within world bounds refers to a unique square in the world
	/// * `tile size` - a world can define the geometrical extents of a single square/tile; these can then be used to define the world positions
	/// * `world position` - the real-number, euclidean position inside a world; in general, the originating world position of a tile is given by: `tile_position * tile_size`
	/// 
	/// The `tile space` and `world` concepts may, in some cases, be used interchangeably, as they have similar properties in this context.
	/// In general, a `world` is both the container of all the squares and their data (as the `grid` type represents) and the set of properties (like the tile space) to operate within it.
	/// @{
	
	/// \internal https://www.redblobgames.com/grids/parts/

	template <std::integral T>
	constexpr bool is_surrounding(glm::tvec2<T> a, glm::tvec2<T> b) { const auto d = glm::abs(a - b); return d.x < 2 && d.y < 2 && d.x + d.y > 0; }
	template <std::integral T>
	constexpr bool is_neighbor(glm::tvec2<T> a, glm::tvec2<T> b) { const auto d = glm::abs(a - b); return d.x < 2 && d.y < 2 && d.x != d.y; }
	template <std::integral T>
	constexpr bool is_diagonal_neighbor(glm::tvec2<T> a, glm::tvec2<T> b) { const auto d = glm::abs(a - b); return d.x < 2 && d.y < 2 && d.x == d.y; }

	/// Any type that meets the criteria of a `metric` - specifying the adjacency/distances between squares
	template <typename METRIC, typename T>
	concept metric = std::integral<T> && requires (T t, glm::tvec2<T> vec) {
		{ METRIC::is_adjacent(vec, vec) } -> std::convertible_to<bool>;
		{ METRIC::is_valid_direction(direction{}) } -> std::convertible_to<bool>;
		{ METRIC::distance(vec, vec) } -> std::convertible_to<T>;
	};

	/// Metric in which only neighbors are adjacent
	struct manhattan_metric
	{
		template <std::integral T>
		static constexpr auto is_adjacent(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			return is_neighbor(a, b);
		}

		static constexpr auto is_valid_direction(direction dir)
		{
			return is_cardinal(dir);
		}

		template <std::integral T>
		static constexpr auto distance(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			const auto d = glm::abs(b - a);
			return d.x + d.y;
		}
	};
	using neighbor_metric = manhattan_metric;

	/// Metric in which the entire surrounding is adjacent
	struct chebyshev_metric
	{
		template <std::integral T>
		static constexpr bool is_adjacent(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			return is_surrounding(a, b);
		}

		static constexpr bool is_valid_direction(direction dir)
		{
			return is_valid(dir);
		}

		template <std::integral T>
		static constexpr auto distance(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			const auto d = glm::abs(b - a);
			return glm::max(d.x, d.y);
		}
	};
	using surrounding_metric = chebyshev_metric;

	/// \internal TODO: Is there a reason this needs to be integral only?
	template <std::integral T>
	constexpr auto manhattan_distance(glm::tvec2<T> a, glm::tvec2<T> b)
	{
		return manhattan_metric::distance(a, b);
	}
	template <std::integral T>
	constexpr auto neighbor_distance(glm::tvec2<T> a, glm::tvec2<T> b)
	{
		return neighbor_metric::distance(a, b);
	}

	template <std::integral T>
	constexpr auto chebyshev_distance(glm::tvec2<T> a, glm::tvec2<T> b)
	{
		return chebyshev_metric::distance(a, b);
	}
	template <std::integral T>
	constexpr auto surrounding_distance(glm::tvec2<T> a, glm::tvec2<T> b)
	{
		return surrounding_metric::distance(a, b);
	}

	constexpr glm::vec2 tile_pos_to_world_pos(glm::ivec2 tile_pos, glm::vec2 tile_size) { return glm::vec2(tile_pos) * tile_size; }
	constexpr glm::vec2 tile_pos_to_world_pos(glm::ivec2 tile_pos, float tile_size) { return glm::vec2(tile_pos) * tile_size; }
	constexpr rec2 world_rect_for_tile(glm::ivec2 pos, glm::vec2 tile_size) { return rec2::from_size(tile_pos_to_world_pos(pos, tile_size), tile_size); }
	constexpr rec2 world_rect_for_tile(glm::ivec2 pos, float tile_size) { return rec2::from_size(tile_pos_to_world_pos(pos, tile_size), { tile_size, tile_size }); }

	inline glm::ivec2 world_pos_to_tile_pos(glm::vec2 world_pos, glm::vec2 tile_size) { return glm::ivec2(glm::floor(world_pos / tile_size)); }
	inline glm::ivec2 world_pos_to_tile_pos(glm::vec2 world_pos, float tile_size) { return glm::ivec2(glm::floor(world_pos / tile_size)); }
	inline irec2 world_rect_to_tile_rect(rec2 const& world_rect, glm::vec2 tile_size) { return irec2{ glm::floor(world_rect.p1 / tile_size), glm::ceil(world_rect.p2 / tile_size) }; }
	inline irec2 world_rect_to_tile_rect(rec2 const& world_rect, float tile_size) { return irec2{ glm::floor(world_rect.p1 / tile_size), glm::ceil(world_rect.p2 / tile_size) }; }

	inline glm::vec2 snap_world_pos_to_tile_grid(glm::vec2 world_pos, glm::vec2 tile_size) { 
		return glm::floor((world_pos + (tile_size * 0.5f)) / tile_size) * tile_size;
	}

	/// Combines a metric and tile size into a single type
	template <metric<float> METRIC = chebyshev_metric>
	struct tile_space
	{
		glm::vec2 tile_size;

		constexpr glm::vec2 to_world_pos(glm::ivec2 tile_pos) const noexcept { return tile_pos_to_world_pos(tile_pos, tile_size); }
		constexpr rec2 world_rect_for_tile(glm::ivec2 tile_pos) const noexcept { return ghassanpl::geometry::squares::world_rect_for_tile(tile_pos, tile_size); }
		constexpr glm::ivec2 to_tile_pos(glm::vec2 world_pos) const noexcept { return world_pos_to_tile_pos(world_pos, tile_size); }
		constexpr irec2 to_tile_rect(rec2 const& world_rect) const noexcept { return world_rect_to_tile_rect(world_rect, tile_size); }
		
		inline glm::vec2 snap_to_grid(glm::vec2 world_pos) const { return snap_world_pos_to_tile_grid(world_pos, tile_size); }
	};

	/// Tile position with `int` values
	using tile_pos = named<glm::ivec2, "tile_pos", traits::location>;
	/// World position with `float` values
	using world_pos = named<glm::vec2, "world_pos", traits::location>;

	/// Tile rectangle with `int` values
	using tile_rec = named<irec2, "tile_rec">;
	/// World rectangle with `float` values
	using world_rec = named<rec2, "world_rec">;

	/// @}
}