/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "square_grid.h"
#include <queue>

namespace ghassanpl::geometry::squares
{
	template <typename TILE_DATA, bool RESIZABLE, typename FUNC>
	void apply_cellular_automata(grid<TILE_DATA, RESIZABLE>& current_iteration, irec2 const& rect, FUNC&& func)
	{
		if constexpr (std::invocable<FUNC, TILE_DATA&, std::span<glm::ivec2 const>>)
		{
			using iteration_flags = grid<TILE_DATA, RESIZABLE>::iteration_flags;
			static constexpr enum_flags<iteration_flags> neighbor_iteration_flags = { iteration_flags::only_valid, iteration_flags::diagonals };

			grid<TILE_DATA, RESIZABLE> previous_iteration = current_iteration;

			std::vector<glm::ivec2> neighbors;
			current_iteration.for_each_tile_in_rect(rect, [&](glm::ivec2 pos) {
				neighbors.clear();
				current_iteration.for_each_neighbor<neighbor_iteration_flags>(pos, op::push_back_to(neighbors));
				func(previous_iteration[pos], std::span<glm::ivec2 const>{ neighbors });
			});

			std::swap(current_iteration, previous_iteration);
		}
		else if constexpr (std::invocable<FUNC, TILE_DATA&, std::span<TILE_DATA const* const>>)
		{
			apply_cellular_automata(current_iteration, rect, [&, func = std::forward<FUNC>(func)](TILE_DATA& cell, std::span<glm::ivec2 const> neighbors){
				std::array<TILE_DATA const*, 8> neighbor_cells;
				for (size_t i = 0; i < neighbors.size(); ++i)
					neighbor_cells[i] = &current_iteration[neighbors[i]];
				func(cell, std::span<TILE_DATA const* const>{&neighbor_cells[0], neighbors.size()});
			});
		}
		else
		{
			static_assert(std::is_same_v<TILE_DATA, FUNC>);
		}
	}

	template <typename TILE_DATA, bool RESIZABLE, typename FUNC>
	void apply_cellular_automata(grid<TILE_DATA, RESIZABLE>& current_iteration, FUNC&& func)
	{
		apply_cellular_automata(current_iteration, current_iteration.bounds(), std::forward<FUNC>(func));
	}


	template <typename TILE_DATA, bool RESIZABLE, change_tile_callback<TILE_DATA> FLOOD_FUNC, query_tile_callback<TILE_DATA> SHOULD_FLOOD_FUNC>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, FLOOD_FUNC&& flood, SHOULD_FLOOD_FUNC&& should_flood)
	{
		std::queue<glm::ivec2> queue;
		if (!grid.is_valid(start)) return;
		if (!should_flood(start, *grid.at(start))) return;

		/// TODO: We could probably use grid.apply<> to call `should_flood`
		 
		queue.push(start);
		while (!queue.empty())
		{
			auto n = queue.front();
			queue.pop();

			auto l = n, r = glm::ivec2{ n.x + 1, n.y };

			while (grid.is_valid(l) && should_flood(l, *grid.at(l)))
				l.x--;
			l.x++;

			while (grid.is_valid(r) && should_flood(r, *grid.at(r)))
				r.x++;
			r.x--;

			for (int x = l.x; x <= r.x; x++)
			{
				glm::ivec2 pos{ x, n.y };
				flood(pos, *grid.at(pos));

				glm::ivec2 up = { pos.x, pos.y - 1 }, down = { pos.x, pos.y + 1 };

				if (grid.is_valid(up) && should_flood(up, *grid.at(up)))
					queue.push(up);

				if (grid.is_valid(down) && should_flood(down, *grid.at(down)))
					queue.push(down);
			}
		}
	}

	template <typename TILE_DATA, bool RESIZABLE, change_tile_callback<TILE_DATA> FLOOD_FUNC>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, FLOOD_FUNC&& flood)
	{
		const auto data_at_start = grid.at(start);
		flood_at(grid, start, std::forward<FLOOD_FUNC>(flood), [&](glm::ivec2 at, TILE_DATA const& data) { return data == data_at_start; });
	}


	template <typename TILE_DATA, bool RESIZABLE, query_tile_callback<TILE_DATA> SHOULD_FLOOD_FUNC>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, TILE_DATA const& replace_with, SHOULD_FLOOD_FUNC&& should_flood)
	{
		flood_at(grid, start, [&](glm::ivec2 at, TILE_DATA& data) { data = replace_with; }, std::forward<SHOULD_FLOOD_FUNC>(should_flood));
	}

	template <typename TILE_DATA, bool RESIZABLE>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, TILE_DATA const& replace_with)
	{
		flood_at(grid, start, [&](glm::ivec2 at, TILE_DATA& data) { data = replace_with; });
	}

}