/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "square_grid.h"
#include <queue>

namespace ghassanpl::geometry::squares
{
	/// \ingroup Grid
	/// @{

	/// Applies a cellular automata generation function on (a region of) the grid.
	/// Note that since cellular automata operations are done on an immutable grid, this function creates a copy of the entire grid
	/// as the next generation, modifies it, and then swaps it with the current generation grid. Any iterators to the current (old) grid
	/// will therefore be invalidated and its data destroyed.
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
				current_iteration.template for_each_neighbor<neighbor_iteration_flags>(pos, op::push_back_to(neighbors));
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

	/// \copydoc apply_cellular_automata
	template <typename TILE_DATA, bool RESIZABLE, typename FUNC>
	void apply_cellular_automata(grid<TILE_DATA, RESIZABLE>& current_iteration, FUNC&& func)
	{
		apply_cellular_automata(current_iteration, current_iteration.bounds(), std::forward<FUNC>(func));
	}

	/// TODO: This
	template <typename TILE_DATA, bool RESIZABLE, typename FUNC>
	void apply_cellular_automata(grid<TILE_DATA, RESIZABLE> const& current_iteration, grid<TILE_DATA, RESIZABLE>& new_iteration, irec2 const& rect, FUNC&& func);

	/// Performs a flood-fill algorithm on `grid`
	/// \param start where to start flooding; `should_flood(start)` should be `true` before `replace(start)`, and `false` afterwards
	/// \param replace will be called on each tile that needs to be flooded, to change it; after `replace(pos)`, `should_flood(pos)` should always be false
	/// \param should_flood queries whether the given tile should be flooded
	template <typename TILE_DATA, bool RESIZABLE, change_tile_callback<TILE_DATA> REPLACE_FUNC, query_tile_callback<TILE_DATA> SHOULD_FLOOD_FUNC>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, REPLACE_FUNC&& replace, SHOULD_FLOOD_FUNC&& should_flood)
	{
		if (!grid.is_valid(start)) return;
		if (!grid.apply(start, should_flood)) return;

		grid.apply(start, replace);
		if (grid.apply(start, should_flood)) return; /// Checks for degenerate case where we're flooding with the same thing that was at origin

		std::queue<glm::ivec2> queue;
		queue.push(start);
		while (!queue.empty())
		{
			auto const n = queue.front();
			queue.pop();

			auto l = n, r = glm::ivec2{ n.x + 1, n.y };

			while (grid.is_valid(l) && grid.apply(l, should_flood))
			{
				grid.apply(l, replace);
				l.x--;
			}
			l.x++;

			while (grid.is_valid(r) && grid.apply(r, should_flood))
			{
				grid.apply(r, replace);
				r.x++;
			}
			r.x--;

			if (n.y - 1 >= 0)
			{
				bool span_added = false;
				for (int x = l.x; x <= r.x; x++)
				{
					glm::ivec2 up = { x, n.y - 1 };
					if (grid.apply(up, should_flood))
					{
						if (!span_added)
						{
							span_added = true;
							queue.push(up);
						}
					}
					else
						span_added = false;
				}
			}
			if (n.y + 1 < grid.height())
			{
				bool span_added = false;
				for (int x = l.x; x <= r.x; x++)
				{
					glm::ivec2 down = { x, n.y + 1 };
					if (grid.apply(down, should_flood))
					{
						if (!span_added)
						{
							span_added = true;
							queue.push(down);
						}
					}
					else
						span_added = false;
				}
			}
		}
		//*/
	}

	/// Performs a flood-fill algorithm on `grid`.
	/// Whether a tile should be flooded will depend on if it compares equal to a **copy** of the tile at `start`
	/// \param start where to start flooding
	/// \param replace will be called on each tile that needs to be flooded, to change it
	template <typename TILE_DATA, bool RESIZABLE, change_tile_callback<TILE_DATA> REPLACE_FUNC>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, REPLACE_FUNC&& replace)
	{
		static_assert(std::equality_comparable<TILE_DATA>, "To use this flood fill algorithm, tile data in this grid must be comparable using operator==");
		const auto* data_at_start = grid.at(start);
		flood_at(grid, start, std::forward<REPLACE_FUNC>(replace), [data_at_start](glm::ivec2 at, TILE_DATA const& data) { return data == *data_at_start; });
	}

	/// Performs a flood-fill algorithm on `grid`.
	/// \param start where to start flooding; `should_flood(start)` should be `true` before `*grid.at(start) = replace_with`, and `false` afterwards
	/// \param replace_with the tile data to set on each square that should be flooded
	/// \param should_flood queries whether the given tile should be flooded
	template <typename TILE_DATA, bool RESIZABLE, query_tile_callback<TILE_DATA> SHOULD_FLOOD_FUNC>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, TILE_DATA const& replace_with, SHOULD_FLOOD_FUNC&& should_flood)
	{
		static_assert(std::is_copy_assignable_v<TILE_DATA>, "To use this flood fill algorithm, tile data in this grid must be copy-assignable");
		flood_at(grid, start, [&](glm::ivec2 at, TILE_DATA& data) { data = replace_with; }, std::forward<SHOULD_FLOOD_FUNC>(should_flood));
	}

	/// Performs a flood-fill algorithm on `grid`.
	/// Whether a tile should be flooded will depend on if it compares equal to a **copy** of the tile at `start`
	/// \param start where to start flooding
	/// \param replace_with the tile data to set on each square that should be flooded
	template <typename TILE_DATA, bool RESIZABLE>
	void flood_at(grid<TILE_DATA, RESIZABLE>& grid, glm::ivec2 start, TILE_DATA const& replace_with)
	{
		static_assert(std::is_copy_assignable_v<TILE_DATA>, "To use this flood fill algorithm, tile data in this grid must be copy-assignable");
		flood_at(grid, start, [&](glm::ivec2 at, TILE_DATA& data) { data = replace_with; });
	}

	/// @}
}