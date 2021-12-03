/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

namespace ghassanpl
{

	///************************************************************************/
	/// Align
	///************************************************************************/

	enum class horizontal_align
	{
		left = 0,
		center = 1, centre = 1,
		right = 2,
		justify = 3,
	};

	enum class vertical_align
	{
		top = 0,
		middle = 4, center = 4, centre = 4,
		bottom = 8,
		justify = 12,
	};

	enum class align;

	constexpr inline align operator|(horizontal_align first, vertical_align second) { return align(int(first) | int(second)); }
	constexpr inline align operator|(vertical_align first, horizontal_align second) { return align(int(first) | int(second)); }

	enum class align
	{
		top_left = int(horizontal_align::left | vertical_align::top),
		top_center = int(horizontal_align::center | vertical_align::top), top_centre = top_center, center_top = top_center, centre_top = top_center,
		top_right = int(horizontal_align::right | vertical_align::top),

		middle_left = int(horizontal_align::left | vertical_align::middle), center_left = middle_left, centre_left = middle_left,
		middle_center = int(horizontal_align::center | vertical_align::middle), center = middle_center, centre = middle_center,
		middle_right = int(horizontal_align::right | vertical_align::middle), center_right = middle_right, centre_right = middle_right,

		bottom_left = int(horizontal_align::left | vertical_align::bottom),
		bottom_center = int(horizontal_align::center | vertical_align::bottom), bottom_centre = bottom_center, center_bottom = bottom_center, centre_bottom = bottom_center,
		bottom_right = int(horizontal_align::right | vertical_align::bottom),
	};

	constexpr inline align aligns_in_order[] = { align::top_left, align::top_center, align::top_right, align::middle_left, align::middle_center, align::middle_right, align::bottom_left, align::bottom_center, align::bottom_right };
	constexpr inline align aligns_clockwise[] = { align::top_left, align::top_center, align::top_right, align::middle_right, align::bottom_right, align::bottom_center, align::bottom_left, align::middle_left };
	constexpr inline align aligns_counter_clockwise[] = { align::top_left, align::middle_left, align::bottom_left, align::bottom_center, align::bottom_right, align::middle_right, align::top_right, align::top_center };

	constexpr inline horizontal_align horizontal_aligns_in_order[] = { horizontal_align::left, horizontal_align::center, horizontal_align::right };
	constexpr inline vertical_align vertical_aligns_in_order[] = { vertical_align::top, vertical_align::middle, vertical_align::bottom };

	constexpr inline align& operator|=(align& first, horizontal_align second) { return first = align(int(first) | int(second)); }
	constexpr inline align& operator|=(align& first, vertical_align second) { return first = align(int(first) | int(second)); }

	constexpr inline const char* horizontal_align_names[] = { "left", "center", "right", "justify_horizontal", "center", "", "", "", "right", "", "", "", "justify_horizontal" };
	constexpr inline const char* vertical_align_names[] = { "top", "middle", "bottom", "justify_vertical", "middle", "", "", "", "bottom", "", "", "", "justify_vertical" };
	constexpr inline const char* align_names[] =         { "top_left", "top_center", "top_right", "", "middle_left", "middle_center", "middle_right", "", "bottom_left", "bottom_center", "bottom_right", "", "" };
	constexpr inline const char* align_names_natural[] = { "top left", "center top", "top right", "", "middle left", "center", "middle right", "", "bottom left", "center bottom", "bottom right", "", "" };

	constexpr inline const char* to_name(horizontal_align h) { return horizontal_align_names[int(h)]; }
	constexpr inline const char* to_name(vertical_align v) { return vertical_align_names[int(v)]; }
	constexpr inline const char* to_name(align a) { return align_names[int(a)]; }
	constexpr inline const char* to_natural_name(align a) { return align_names_natural[int(a)]; }

	namespace detail
	{
		static inline constexpr auto horizontal_align_mask = int(horizontal_align::left) | int(horizontal_align::center) | int(horizontal_align::right);
		static inline constexpr auto vertical_align_mask = int(vertical_align::top) | int(vertical_align::middle) | int(vertical_align::bottom);
	}

	constexpr inline vertical_align to_vertical(horizontal_align alignment) { return vertical_align{ (int(alignment) & detail::horizontal_align_mask) << 2 }; }
	constexpr inline vertical_align to_vertical(vertical_align alignment) { return alignment; }
	constexpr inline horizontal_align to_horizontal(vertical_align alignment) { return horizontal_align{ (int(alignment) & detail::vertical_align_mask) >> 2 }; }
	constexpr inline horizontal_align to_horizontal(horizontal_align alignment) { return alignment; }
	constexpr inline vertical_align vertical_from(align alignment) { return vertical_align{ (int(alignment) & detail::vertical_align_mask) }; }
	constexpr inline horizontal_align horizontal_from(align alignment) { return horizontal_align{ (int(alignment) & detail::horizontal_align_mask) }; }
	constexpr inline align only_vertical(align alignment) { return align{ (int(alignment) & detail::vertical_align_mask) }; }
	constexpr inline align only_horizontal(align alignment) { return align{ (int(alignment) & detail::horizontal_align_mask) }; }

	constexpr inline vertical_align to_opposite(vertical_align alignment) { return vertical_align{ ((2 - (int(alignment) >> 2)) & detail::horizontal_align_mask) << 2 }; }
	constexpr inline horizontal_align to_opposite(horizontal_align alignment) { return horizontal_align{ (2 - int(alignment)) & detail::horizontal_align_mask }; }
	constexpr inline align to_opposite(align alignment) { return to_opposite(vertical_from(alignment)) | to_opposite(horizontal_from(alignment)); }

	constexpr inline align rotated_clockwise(align alignment) { return to_horizontal(to_opposite(vertical_from(alignment))) | to_vertical(horizontal_from(alignment)); }
	constexpr inline align rotated_counter_clockwise(align alignment) { return to_horizontal(vertical_from(alignment)) | to_vertical(to_opposite(horizontal_from(alignment))); }

	constexpr inline align flipped_horizontally(align alignment) { return vertical_from(alignment) | to_opposite(horizontal_from(alignment)); }
	constexpr inline align flipped_vertically(align alignment) { return to_opposite(vertical_from(alignment)) | horizontal_from(alignment); }

	template <typename T>
	inline constexpr T aligned(const T& width, const T& max_width, horizontal_align align) {
		switch (align)
		{
		case horizontal_align::center: return (max_width / 2 - width / 2);
		case horizontal_align::right: return (max_width - width);
		default:
		case horizontal_align::left: return 0;
		}
	}

	template <typename T>
	inline constexpr T aligned(const T& width, const T& max_width, vertical_align align) {
		switch (align)
		{
		case vertical_align::middle: return (max_width / 2 - width / 2);
		case vertical_align::bottom: return (max_width - width);
		default:
		case vertical_align::top: return 0;
		}
	}

	/// Margins

	/*
	template <typename T>
	struct HorizontalMargins
	{
		T Left = {};
		T Right = {};

		constexpr HorizontalMargins(T left, T right) : Left(left), Right(right) {}

		constexpr HorizontalMargins(T container_width, T width, Alignment align)
		{
			switch (GetHorizontal(align))
			{
			case Alignment::Right:
				Left = container_width - width;
				break;
			case Alignment::Center:
				Left = container_width / 2 - detail::DivideRoundDown(width, 2);
				Right = container_width / 2 + detail::DivideRoundUp(width, 2);
				break;
			case Alignment::Left:
			default:
				Left = container_width - width;
				break;
			}
		}

		constexpr HorizontalMargins() = default;
		constexpr HorizontalMargins(const HorizontalMargins&) = default;
		constexpr HorizontalMargins(HorizontalMargins&&) noexcept = default;
		constexpr HorizontalMargins& operator=(const HorizontalMargins&) = default;
		constexpr HorizontalMargins& operator=(HorizontalMargins&&) noexcept = default;
	};
	*/
}