/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "direction.h"

namespace ghassanpl::geometry::squares
{
	/// TODO: Should probably define the terms somewhere (surrounding, neighbor, adjacent, tile, world, etc)
	/// 
	/// https://www.redblobgames.com/grids/parts/
	
	constexpr bool is_surrounding(glm::ivec2 a, glm::ivec2 b) { const auto d = glm::abs(a - b); return d.x < 2 && d.y < 2 && d.x + d.y > 0; }
	constexpr bool is_neighbor(glm::ivec2 a, glm::ivec2 b) { const auto d = glm::abs(a - b); return d.x < 2 && d.y < 2 && d.x != d.y; }
	constexpr bool is_diagonal_neighbor(glm::ivec2 a, glm::ivec2 b) { return !is_neighbor(a, b); }

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
		static constexpr auto is_valid_neighbor(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			return is_neighbor(a, b);
		}

		static constexpr auto is_valid_direction(direction dir)
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
			return glm::max(d.x, d.y);
		}

		template <std::integral T>
		static constexpr bool is_valid_neighbor(glm::tvec2<T> a, glm::tvec2<T> b)
		{
			return is_surrounding(a, b);
		}

		static constexpr bool is_valid_direction(direction dir)
		{
			return is_valid(dir);
		}
	};
	using surrounding_metric = chebyshev_metric;

	/// TODO: Is there a reason this needs to be integral only?
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

	template <metric METRIC = chebyshev_metric>
	struct tile_space
	{
		glm::vec2 tile_size;

		constexpr glm::vec2 to_world_pos(glm::ivec2 tile_pos) const noexcept { return tile_pos_to_world_pos(tile_pos, tile_size); }
		constexpr rec2 world_rect_for_tile(glm::ivec2 tile_pos) const noexcept { return ghassanpl::geometry::squares::world_rect_for_tile(tile_pos, tile_size); }
		constexpr glm::ivec2 to_tile_pos(glm::vec2 world_pos) const noexcept { return world_pos_to_tile_pos(world_pos, tile_size); }
		constexpr irec2 to_tile_rect(rec2 const& world_rect) const noexcept { return world_rect_to_tile_rect(world_rect, tile_size); }
	};

	using tile_pos = named<glm::ivec2, "tile_pos", traits::location>;
	using world_pos = named<glm::vec2, "world_pos", traits::location>;

	using tile_rec = named<irec2, "tile_rec">;
	using world_rec = named<rec2, "world_rec">;
}