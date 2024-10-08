/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "squares.h"
#include <vector>
#include <stdexcept>

namespace ghassanpl::geometry::squares
{
	template <typename T>
	concept void_or_boolean = std::same_as<void, T> || std::convertible_to<T, bool>;
	/*
	template <typename FUNC, typename TILE_DATA>
	concept tile_callback =
		requires (FUNC func) { { func(glm::ivec2{ 0, 0 }) } -> void_or_boolean; } ||
		requires (FUNC func, TILE_DATA& ref) { { func(glm::ivec2{ 0, 0 }, ref) } -> void_or_boolean; } ||
		requires (FUNC func, TILE_DATA& ref) { { func(ref, glm::ivec2{ 0, 0 }) } -> void_or_boolean; } ||
		requires (FUNC func, TILE_DATA& ref) { { func(ref) } -> void_or_boolean; };
		*/
	template <typename FUNC>
	concept tile_callback = requires (FUNC func) { { func(glm::ivec2{ 0, 0 }) } -> void_or_boolean; };
	template <typename FUNC, typename T>
	concept query_tile_callback = requires (FUNC func, T const& td) { { func(glm::ivec2{ 0, 0 }, td) } -> std::convertible_to<bool>; };
	template <typename FUNC, typename T>
	concept change_tile_callback = requires (FUNC func, T& td) { { func(glm::ivec2{ 0, 0 }, td) }; };

	template <typename TILE_DATA, bool RESIZABLE = true>
	struct grid
	{
	public:

		static constexpr bool resizable = RESIZABLE;
		using tile_data_type = TILE_DATA;

		grid() requires RESIZABLE = default;
		grid(int w, int h, TILE_DATA const& default_tile) { Reset(w, h, default_tile); }
		grid(glm::ivec2 size, TILE_DATA const& default_tile) : grid(size.x, size.y, default_tile) {}
		grid(int w, int h) { Reset(w, h); }
		explicit grid(glm::ivec2 size) : grid(size.x, size.y) {}

		void reset(int w, int h, TILE_DATA const& default_tile) requires RESIZABLE { Reset(w, h, default_tile); }
		void reset(int w, int h, std::vector<TILE_DATA> tiles) requires RESIZABLE { Reset(w, h, tiles); }
		void reset(int w, int h) requires RESIZABLE { Reset(w, h); }

		void reset(glm::ivec2 size) requires RESIZABLE { Reset(size.x, size.y); }
		void reset(glm::ivec2 size, TILE_DATA const& default_tile) requires RESIZABLE { Reset(size.x, size.y, default_tile); }

		template <change_tile_callback<TILE_DATA> TILE_RESET_FUNC>
		void resetFrom(glm::ivec2 size, TILE_RESET_FUNC&& tile_reset) requires RESIZABLE { Reset(size.x, size.y, std::forward<TILE_RESET_FUNC>(tile_reset)); }

		/// Accessors & Queries

		[[nodiscard]] TILE_DATA& operator[](int i) { return mTiles[i]; }
		[[nodiscard]] TILE_DATA const& operator[](int i) const { return mTiles[i]; }

#if defined(__cpp_multidimensional_subscript)
		[[nodiscard]] TILE_DATA& operator[](int x, int y) { return mTiles[index(x, y)]; }
		[[nodiscard]] TILE_DATA const& operator[](int x, int y) const { return mTiles[index(x, y)]; }
#endif
		
		[[nodiscard]] TILE_DATA& operator[](glm::ivec2 v) { return mTiles[index(v)]; }
		[[nodiscard]] TILE_DATA const& operator[](glm::ivec2 v) const { return mTiles[index(v)]; }
		
		[[nodiscard]] bool is_valid(int x, int y) const noexcept { return x >= 0 && y >= 0 && x < mWidth && y < mHeight; }
		[[nodiscard]] bool is_valid(glm::vec2 world_pos, glm::vec2 tile_size) const noexcept { return is_valid(world_pos_to_tile_pos(world_pos, tile_size)); }
		[[nodiscard]] bool is_valid(glm::ivec2 pos) const noexcept { return is_valid(pos.x, pos.y); }
		[[nodiscard]] bool is_index_valid(int index) const noexcept { return index >= 0 && index < (int)mTiles.size(); }
		
		[[nodiscard]] bool is_valid(int x, int y, int edge_width) const noexcept { return x >= edge_width && y >= edge_width && x < mWidth - edge_width && y < mHeight - edge_width; }
		[[nodiscard]] bool is_valid(glm::vec2 world_pos, glm::vec2 tile_size, int edge_width) const noexcept { return is_valid(world_pos_to_tile_pos(world_pos, tile_size), edge_width); }
		[[nodiscard]] bool is_valid(glm::ivec2 pos, int edge_width) const noexcept { return is_valid(pos.x, pos.y, edge_width); }
		
		[[nodiscard]] inline int index(int x, int y) const noexcept { return x + y * mWidth; }
		[[nodiscard]] inline int index(glm::ivec2 pos) const noexcept { return pos.x + pos.y * mWidth; }
		[[nodiscard]] inline int valid_index(int x, int y) const noexcept { return is_valid(x, y) ? x + y * mWidth : -1; }
		[[nodiscard]] inline int valid_index(glm::ivec2 pos) const noexcept { return is_valid(pos) ? pos.x + pos.y * mWidth : -1; }
		
		[[nodiscard]] TILE_DATA const* at(glm::ivec2 pos) const noexcept { if (!is_valid(pos)) return nullptr; return &mTiles[index(pos)]; }
		[[nodiscard]] TILE_DATA const* at(int x, int y) const noexcept { return at(glm::ivec2{ x, y }); }
		[[nodiscard]] TILE_DATA* at(glm::ivec2 pos) noexcept { if (!is_valid(pos)) return nullptr; return &mTiles[index(pos)]; }
		[[nodiscard]] TILE_DATA* at(int x, int y) noexcept { return at(glm::ivec2{ x, y }); }
		[[nodiscard]] TILE_DATA const* at_index(int index) const noexcept { return is_index_valid(index) ? &mTiles[index] : nullptr; }
		[[nodiscard]] TILE_DATA* at_index(int index) noexcept { return is_index_valid(index) ? &mTiles[index] : nullptr; }
		
		[[nodiscard]] TILE_DATA const& safe_at(glm::ivec2 pos, TILE_DATA const& outside) const noexcept { if (auto at = this->at(pos)) return *at; return outside; }
		[[nodiscard]] TILE_DATA const& safe_at(int x, int y, TILE_DATA const& outside) const noexcept { return safe_at(glm::ivec2{ x, y }, outside); }
		[[nodiscard]] TILE_DATA& safe_at(glm::ivec2 pos, TILE_DATA& outside) noexcept { if (auto at = this->at(pos)) return *at; return outside; }
		[[nodiscard]] TILE_DATA& safe_at(int x, int y, TILE_DATA& outside) noexcept { return safe_at(glm::ivec2{ x, y }, outside); }
		
		[[nodiscard]] int width() const noexcept { return mWidth; }
		[[nodiscard]] int height() const noexcept { return mHeight; }
		[[nodiscard]] glm::ivec2 size() const noexcept { return { mWidth, mHeight }; }
		[[nodiscard]] irec2 perimeter() const noexcept { return irec2::from_size({}, size()); }
		[[nodiscard]] irec2 bounds() const noexcept { return perimeter(); }
		[[nodiscard]] rec2 bounds(glm::vec2 tile_size) const noexcept { return perimeter() * tile_size; }
		[[nodiscard]] std::span<TILE_DATA const> tiles() const { return mTiles; }
		[[nodiscard]] std::span<TILE_DATA> tiles() { return mTiles; }
		[[nodiscard]] size_t tile_count() const noexcept { return mTiles.size(); }

		/*
		/// https://github.com/nothings/stb/blob/master/stb_connected_components.h
		/// Need to determine whether tile is passable or not (maybe enum_flags::is_set(Definition->Flags, TileFlag::IsPassable) ?)
		bool IsReachable(glm::ivec2 src_tile, glm::ivec2 dest_tile);
		*/

		enum class iteration_flags
		{
			with_self,
			only_valid,
			diagonals
		};

		/// TODO: set_* functions that mirror for_each_*, e.g. set_rect(rect, tile);

		template <enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::with_self, iteration_flags::only_valid }, typename FUNC >
		auto for_each_neighbor(glm::ivec2 of, FUNC&& func)
		{
			static constexpr auto ONLY_VALID = FLAGS.contain(iteration_flags::only_valid);
			using return_type = decltype(this->apply<ONLY_VALID>(glm::ivec2{ 0, 0 }, func));

			if constexpr (std::is_void_v<return_type>)
			{
				if constexpr (FLAGS.contain(iteration_flags::with_self))
					apply<ONLY_VALID>(of, func);
				apply<ONLY_VALID>({ of.x - 1, of.y }, func);
				apply<ONLY_VALID>({ of.x + 1, of.y }, func);
				apply<ONLY_VALID>({ of.x, of.y - 1 }, func);
				apply<ONLY_VALID>({ of.x, of.y + 1 }, func);

				if constexpr (FLAGS.contain(iteration_flags::diagonals))
				{
					apply<ONLY_VALID>({ of.x - 1, of.y - 1 }, func);
					apply<ONLY_VALID>({ of.x + 1, of.y + 1 }, func);
					apply<ONLY_VALID>({ of.x + 1, of.y - 1 }, func);
					apply<ONLY_VALID>({ of.x - 1, of.y + 1 }, func);
				}
			}
			else
			{
				if constexpr (FLAGS.contain(iteration_flags::with_self))
					if (auto ret = apply<ONLY_VALID>(of, func)) return ret;
				if (auto ret = apply<ONLY_VALID>({ of.x - 1, of.y }, func)) return ret;
				if (auto ret = apply<ONLY_VALID>({ of.x + 1, of.y }, func)) return ret;
				if (auto ret = apply<ONLY_VALID>({ of.x, of.y - 1 }, func)) return ret;
				if (auto ret = apply<ONLY_VALID>({ of.x, of.y + 1 }, func)) return ret;

				if constexpr (FLAGS.contain(iteration_flags::diagonals))
				{
					if (auto ret = apply<ONLY_VALID>({ of.x - 1, of.y - 1 }, func)) return ret;
					if (auto ret = apply<ONLY_VALID>({ of.x + 1, of.y + 1 }, func)) return ret;
					if (auto ret = apply<ONLY_VALID>({ of.x + 1, of.y - 1 }, func)) return ret;
					if (auto ret = apply<ONLY_VALID>({ of.x - 1, of.y + 1 }, func)) return ret;
				}
				return return_type{};
			}
		}

		template <enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::with_self, iteration_flags::only_valid }, typename FUNC >
		auto for_each_selected_neighbor(glm::ivec2 of, direction_set neighbor_set, FUNC&& func)
		{
			static constexpr auto ONLY_VALID = FLAGS.contain(iteration_flags::only_valid);
			using return_type = decltype(this->apply<ONLY_VALID>(glm::ivec2{ 0, 0 }, func));

			if constexpr (std::is_void_v<return_type>)
			{
				if constexpr (FLAGS.contain(iteration_flags::with_self))
					apply<ONLY_VALID>(of, func);

				neighbor_set.for_each([this, of, &func](direction d) {
					apply<ONLY_VALID>(of + to_ivec(d), func);
				});
			}
			else
			{
				if constexpr (FLAGS.contain(iteration_flags::with_self))
					if (auto ret = apply<ONLY_VALID>(of, func)) return ret;

				return neighbor_set.for_each([this, of, &func](direction d) {
					return apply<ONLY_VALID>(of + to_ivec(d), func);
				});
			}
		}

		/// TODO: deduce thos
		template <enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::only_valid }, typename FUNC>
		auto for_each_tile_in_rect(irec2 const& tile_rect, FUNC&& func)
		{
			static constexpr auto ONLY_VALID = FLAGS.contain(iteration_flags::only_valid);
			using return_type = decltype(this->template apply<ONLY_VALID>(glm::ivec2{ 0, 0 }, func));

			irec2 rect = tile_rect;
			if constexpr (ONLY_VALID)
				rect = tile_rect.clipped_to(this->bounds());

			for (int y = rect.top(); y < rect.bottom(); y++)
				for (int x = rect.left(); x < rect.right(); x++)
				{
					if constexpr (std::is_void_v<return_type>)
						this->template apply<ONLY_VALID>({ x, y }, func);
					else
					{
						if (auto ret = this->template apply<ONLY_VALID>({ x, y }, func))
							return ret;
					}
				}

			return default_value<return_type>();
		}

		template <enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::only_valid }, typename FUNC >
		auto for_each_tile_in_perimeter(irec2 const& tile_rect, FUNC&& func)
		{
			static constexpr auto ONLY_VALID = FLAGS.contain(iteration_flags::only_valid);
			using return_type = decltype(this->apply<ONLY_VALID>(glm::ivec2{ 0, 0 }, func));

			irec2 rect = tile_rect;
			if constexpr (ONLY_VALID)
				rect = tile_rect.clipped_to(bounds());

			if constexpr (std::is_void_v<return_type>)
			{
				for (int x = rect.left(); x < rect.right(); x++)
				{
					apply<ONLY_VALID>({ x, rect.top() }, func);
					apply<ONLY_VALID>({ x, rect.bottom() - 1 }, func);
				}
				for (int y = rect.top() + 1; y < rect.bottom() - 1; y++)
				{
					apply<ONLY_VALID>({ rect.left(), y }, func);
					apply<ONLY_VALID>({ rect.right() - 1, y }, func);
				}
			}
			else
			{
				for (int x = rect.left(); x < rect.right(); x++)
				{
					if (auto ret = apply<ONLY_VALID>({ x, rect.top() }, func)) return ret;
					if (auto ret = apply<ONLY_VALID>({ x, rect.bottom() - 1 }, func)) return ret;
				}
				for (int y = rect.top() + 1; y < rect.bottom() - 1; y++)
				{
					if (auto ret = apply<ONLY_VALID>({ rect.left(), y }, func)) return ret;
					if (auto ret = apply<ONLY_VALID>({ rect.right() - 1, y }, func)) return ret;
				}

				return return_type{};
			}
		}

		template<enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::only_valid }, typename TILE_SET, typename FUNC >
		auto for_each_tile_in_set(TILE_SET&& tiles, FUNC&& func)
		{
			static constexpr auto ONLY_VALID = FLAGS.contain(iteration_flags::only_valid);
			using return_type = decltype(this->apply<ONLY_VALID>(glm::ivec2{ 0, 0 }, func));

			if constexpr (std::is_void_v<return_type>)
			{
				for (auto&& tile : tiles)
					apply<ONLY_VALID>(tile, func);
			}
			else
			{
				for (auto&& tile : tiles)
					if (auto ret = apply<ONLY_VALID>(tile, func)) return ret;
				return return_type{};
			}
		}

		template <typename FUNC>
		auto for_each_tile(FUNC&& func)
		{
			irec2 rect = { 0, 0, this->mWidth, this->mHeight };
			return this->template for_each_tile_in_rect<enum_flags<iteration_flags>{}>(rect, std::forward<FUNC>(func));
		}

		template <enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::only_valid }, typename FUNC>
		auto for_each_tile_in_polygon(std::span<glm::vec2 const> poly_points, glm::vec2 tile_size, FUNC&& func);

		template <enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::only_valid }, typename FUNC>
		auto for_each_tile_in_row(int row, FUNC&& func);

		template <enum_flags<iteration_flags> FLAGS = enum_flags<iteration_flags>{ iteration_flags::only_valid }, typename FUNC>
		auto for_each_tile_in_column(int column, FUNC&& func);

		/// Function: line_cast
		/// Return: Whether the line between `start` and `end` is free of blocing tiles, as determined by `blocks_func`
		template <typename FUNC>
		bool line_cast(glm::ivec2 start, glm::ivec2 end, FUNC&& blocks_func, bool ignore_start) const
		{
			int delta_x{ end.x - start.x };
			// if x1 == x2, then it does not matter what we set here
			signed char const ix((delta_x > 0) - (delta_x < 0));
			delta_x = std::abs(delta_x) << 1;

			int delta_y(end.y - start.y);
			// if y1 == y2, then it does not matter what we set here
			signed char const iy((delta_y > 0) - (delta_y < 0));
			delta_y = std::abs(delta_y) << 1;

			if (!ignore_start && blocks_func(start))
				return false;

			if (delta_x >= delta_y)
			{
				// error may go below zero
				int error(delta_y - (delta_x >> 1));

				while (start.x != end.x)
				{
					// reduce error, while taking into account the corner case of error == 0
					if ((error > 0) || (!error && (ix > 0)))
					{
						error -= delta_x;
						start.y += iy;
					}
					// else do nothing

					error += delta_y;
					start.x += ix;

					if (blocks_func(start))
						return false;
				}
			}
			else
			{
				// error may go below zero
				int error(delta_x - (delta_y >> 1));

				while (start.y != end.y)
				{
					// reduce error, while taking into account the corner case of error == 0
					if ((error > 0) || (!error && (iy > 0)))
					{
						error -= delta_y;
						start.x += ix;
					}
					// else do nothing

					error += delta_x;
					start.y += iy;

					if (blocks_func(start))
						return false;
				}
			}

			return true;
		}

		/// Modifiers

		template <bool ONLY_VALID = true, typename FUNC>
		auto apply(glm::ivec2 to, FUNC&& func)
		{
			//if constexpr (ONLY_VALID) if (!is_valid(to)) return return_type{};
			using self_type = std::remove_reference_t<decltype(*this)>;
			using tile_data_type = std::conditional_t<std::is_const_v<self_type>, std::add_const_t<typename self_type::tile_data_type>, typename self_type::tile_data_type>;
			using invocable_type = std::remove_cvref_t<FUNC>;
			if constexpr (std::invocable<invocable_type, glm::ivec2, tile_data_type&>)
				return (!ONLY_VALID || this->is_valid(to)) ? func(to, *this->at(to)) : default_value<decltype(func(to, *this->at(to)))>();
			else if constexpr (std::invocable<invocable_type, tile_data_type&, glm::ivec2>)
				return (!ONLY_VALID || this->is_valid(to)) ? func(*this->at(to), to) : default_value<decltype(func(*this->at(to), to))>();
			else if constexpr (std::invocable<invocable_type, tile_data_type&>)
				return (!ONLY_VALID || this->is_valid(to)) ? func(*this->at(to)) : default_value<decltype(func(*this->at(to)))>();
			else
				return (!ONLY_VALID || this->is_valid(to)) ? func(to) : default_value<decltype(func(to))>();
		}

		void flip_row(int row)
		{
			if (row >= 0 && row < mHeight)
				std::reverse(GetRowStart(row), GetRowStart(row) + mWidth);
		}

		void flip_horizontal()
		{
			for (int i = 0; i < mHeight; i++)
				std::reverse(GetRowStart(i), GetRowStart(i) + mWidth);
		}

		void flip_vertical()
		{
			for (int i = 0; i < mHeight / 2; i++)
				std::swap_ranges(GetRowStart(i), GetRowStart(i) + mWidth, GetRowStart(mHeight - i - 1));
		}

		void rotate_row(int row, int by);
		void rotate_column(int column, int by);

		void shift_row(int row, int by, TILE_DATA const& add_tile);
		void shift_column(int column, int by, TILE_DATA const& add_tile);

		void rotate_rows(int by);
		void rotate_columns(int by);

		void shift_rows(int by, TILE_DATA const& add_tile)
		{
			if (by == 0) return;
			else if (by > 0)
			{
				auto num_elems_to_shift = by * mWidth;
				std::move_backward(mTiles.begin(), mTiles.begin() + (mTiles.size() - num_elems_to_shift), mTiles.end());
				std::ranges::fill_n(mTiles.begin(), num_elems_to_shift, add_tile);
			}
			else if (by < 0)
			{
				auto num_elems_to_shift = -by * mWidth;
				std::move(mTiles.begin() + num_elems_to_shift, mTiles.end(), mTiles.begin());
				std::ranges::fill_n(mTiles.begin() + (mTiles.size() - num_elems_to_shift), num_elems_to_shift, add_tile);
			}
		}
		void shift_columns(int by, TILE_DATA const& add_tile);

		void rotate_180() requires RESIZABLE
		{
			for (int i = 0; i < mHeight / 2; i++)
				std::swap_ranges(std::make_reverse_iterator(GetRowStart(i) + mWidth), std::make_reverse_iterator(GetRowStart(i)), GetRowStart(mHeight - i - 1));

			/// Need to reverse middle row if height is odd
			if (mHeight % 1)
				std::reverse(GetRowStart(mHeight / 2 + 1), GetRowStart(mHeight / 2 + 1) + mWidth);
		}

		void resize(glm::uvec2 new_size, const TILE_DATA& new_element) requires RESIZABLE
		{
			ResizeY(new_size.y, new_element);
			ResizeX(new_size.x, new_element);
		}

		void resize(glm::uvec2 new_size) requires RESIZABLE
		{
			const TILE_DATA new_element{};
			resize(new_size, new_element);
		}

		void resize(int w, int h) requires RESIZABLE
		{
			if (w < 0) throw std::invalid_argument{ "width cannot be negative" };
			if (h < 0) throw std::invalid_argument{ "height cannot be negative" };
			resize({ w, h });
		}

		void clear() { for (auto& tile : mTiles) tile = {}; }
		void clear(TILE_DATA const& to) { for (auto& tile : mTiles) tile = to; }

	protected:

		auto GetRowStart(int row) { return mTiles.begin() + row * mWidth; }
		auto GetTileIterator(int x, int y) { return mTiles.begin() + index(x, y); }

		void Reset(int w, int h, TILE_DATA const& default_tile)
		{
			if (w < 0) throw std::invalid_argument{ "width cannot be negative" };
			if (h < 0) throw std::invalid_argument{ "height cannot be negative" };

			mTiles.clear();
			mWidth = w;
			mHeight = h;
			mTiles.resize(w * h, default_tile);
		}

		template <change_tile_callback<TILE_DATA> TILE_RESET_FUNC>
		void Reset(int w, int h, TILE_RESET_FUNC&& tile_reset)
		{
			if (w < 0) throw std::invalid_argument{ "width cannot be negative" };
			if (h < 0) throw std::invalid_argument{ "height cannot be negative" };

			mTiles.clear();
			mWidth = w;
			mHeight = h;
			mTiles.resize(w * h);
			for_each_tile(tile_reset);
			//for_each_tile([&tile_reset](glm::ivec2 t, TILE_DATA& tile) { tile_reset(); });
		}

		void Reset(int w, int h)
		{
			if (w < 0) throw std::invalid_argument{ "width cannot be negative" };
			if (h < 0) throw std::invalid_argument{ "height cannot be negative" };

			mTiles.clear();
			mWidth = w;
			mHeight = h;
			mTiles.resize(w * h);
		}
		
		void Reset(int w, int h, std::vector<TILE_DATA> tiles)
		{
			if (w < 0) throw std::invalid_argument{ "width cannot be negative" };
			if (h < 0) throw std::invalid_argument{ "height cannot be negative" };
			if (size_t(w * h) > tiles.size()) throw std::invalid_argument{ "not enough tiles in vector" };

			tiles.resize(w * h);
			mTiles = std::move(tiles);
			mWidth = w;
			mHeight = h;
		}

		void ResizeY(int new_y, const TILE_DATA& new_element)
		{
			if (new_y < 0) throw std::invalid_argument("new_y cannot be negative");

			const auto new_count = new_y * mWidth;
			mTiles.resize(new_count, new_element);
			mHeight = new_y;
		}

		void ResizeX(int new_x, const TILE_DATA& new_element)
		{
			if (new_x < 0) throw std::invalid_argument("new_x cannot be negative");
			/// TODO: Would it be more cache-friendly to copy to a new vector and swap it with mTiles?
			/*
			std::vector<T> new_vector;
			new_vector.reserve(new_size);
			for (size_t y=0; y<mHeight; ++y)
			{
				for (size_t x=0; x<mSize.x; ++x)
				{
					new_vector.push_back(std::move(mTiles[y * mSize.x + x]));
				}
				for (size_t x=mSize.x; x<new_x; ++x)
				{
					new_vector.push_back(new_element);
				}
			}
			mTiles = std::move(new_vector);
			*/
			const auto new_count = new_x * mHeight;

			if (new_x > mWidth)
			{
				mTiles.resize(new_count, new_element);

				for (int yy = 0; yy < mHeight; ++yy)
				{
					auto y = mHeight - yy - 1;
					const auto begin_range = y * mWidth;
					const auto end_range = begin_range + mWidth;
					const auto new_end_range = y * new_x + mWidth;
					std::swap_ranges(std::make_reverse_iterator(mTiles.begin() + end_range), std::make_reverse_iterator(mTiles.begin() + begin_range), std::make_reverse_iterator(mTiles.begin() + new_end_range));
				}
			}
			else
			{
				const auto dif = mWidth - new_x;
				for (int y = 1; y < mHeight; ++y)
				{
					const auto begin_range = y * mWidth;
					const auto end_range = begin_range + new_x;
					std::move(mTiles.begin() + begin_range, mTiles.begin() + end_range, mTiles.begin() + begin_range - (dif * y));
				}
				mTiles.resize(new_count);
			}

			mWidth = new_x;
		}

		int mWidth = 0;
		int mHeight = 0;
		std::vector<TILE_DATA> mTiles;
	};

}