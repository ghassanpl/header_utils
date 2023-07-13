/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#define GHPL_HAS_ALIGN

namespace ghassanpl
{
	/// \defgroup Alignment Alignment
	/// Types and functions for geometric alignments - relative positionings on a line
	
	/// \ingroup Alignment
	///@{

	/// Represents an alignment on a horizontal line
	enum class horizontal_align
	{
		left = 0, ///< Left
		center = 1, ///< Center
		centre = 1, ///< Same as `center`
		right = 2, ///< Right
		justify = 3, ///< Justify
	};

	/// Represents an alignment on a vertical line
	enum class vertical_align
	{
		top = 0, ///< Top
		middle = 4, ///< Middle
		center = 4, ///< Same as middle
		centre = 4, ///< Same as middle
		bottom = 8, ///< Bottom
		justify = 12, ///< Justify
	};

	enum class align;

	/// You can use the | operator to create an \ref align from a \ref horizontal_align and a \ref vertical_align
	constexpr align operator|(horizontal_align first, vertical_align second) { return align(int(first) | int(second)); }
	/// You can use the | operator to create an \ref align from a \ref horizontal_align and a \ref vertical_align
	constexpr align operator|(vertical_align first, horizontal_align second) { return align(int(first) | int(second)); }
	
	/// Represents an alignment in both axes (horizontal and veritcal)
	enum class align
	{
		/// Top left
		top_left = int(horizontal_align::left | vertical_align::top),
		/// Top center
		top_center = int(horizontal_align::center | vertical_align::top), top_centre = top_center, center_top = top_center, centre_top = top_center,
		/// Top right
		top_right = int(horizontal_align::right | vertical_align::top), 

		/// Middle left
		middle_left = int(horizontal_align::left | vertical_align::middle), center_left = middle_left, centre_left = middle_left,
		/// Middle center
		middle_center = int(horizontal_align::center | vertical_align::middle), center = middle_center, centre = middle_center,
		/// Middle right
		middle_right = int(horizontal_align::right | vertical_align::middle), center_right = middle_right, centre_right = middle_right,

		/// Bottom left
		bottom_left = int(horizontal_align::left | vertical_align::bottom),
		/// Bottom center
		bottom_center = int(horizontal_align::center | vertical_align::bottom), bottom_centre = bottom_center, center_bottom = bottom_center, centre_bottom = bottom_center,
		/// Bottom right
		bottom_right = int(horizontal_align::right | vertical_align::bottom),
	};

	/// Alignments in order, first left-to-right, then top-to-bottom
	constexpr inline align aligns_in_order[] = { align::top_left, align::top_center, align::top_right, align::middle_left, align::middle_center, align::middle_right, align::bottom_left, align::bottom_center, align::bottom_right };
	/// Alignments in clockwise order
	constexpr inline align aligns_clockwise[] = { align::top_left, align::top_center, align::top_right, align::middle_right, align::bottom_right, align::bottom_center, align::bottom_left, align::middle_left };
	/// Alignments in counter-clockwise order
	constexpr inline align aligns_counter_clockwise[] = { align::top_left, align::middle_left, align::bottom_left, align::bottom_center, align::bottom_right, align::middle_right, align::top_right, align::top_center };

	/// Horizontal alignments in order
	constexpr inline horizontal_align horizontal_aligns_in_order[] = { horizontal_align::left, horizontal_align::center, horizontal_align::right };
	/// Vertical alignments in order
	constexpr inline vertical_align vertical_aligns_in_order[] = { vertical_align::top, vertical_align::middle, vertical_align::bottom };

	constexpr align& operator|=(align& first, horizontal_align second) { return first = align(int(first) | int(second)); }
	constexpr align& operator|=(align& first, vertical_align second) { return first = align(int(first) | int(second)); }

	constexpr inline const char* horizontal_align_names[] = { "left", "center", "right", "justify_horizontal", "center", "", "", "", "right", "", "", "", "justify_horizontal" };
	constexpr inline const char* vertical_align_names[] = { "top", "middle", "bottom", "justify_vertical", "middle", "", "", "", "bottom", "", "", "", "justify_vertical" };
	constexpr inline const char* align_names[] =         { "top_left", "top_center", "top_right", "", "middle_left", "middle_center", "middle_right", "", "bottom_left", "bottom_center", "bottom_right", "", "" };
	constexpr inline const char* align_names_natural[] = { "top left", "center top", "top right", "", "middle left", "center", "middle right", "", "bottom left", "center bottom", "bottom right", "", "" };

	/// Get alignment name
	/// \returns Nul-terminated string with the name of the alignment (the same as the enum value name)
	/// \sa horizontal_align_names
	constexpr const char* to_name(horizontal_align h) { return horizontal_align_names[int(h)]; }
	/// Get alignment name
	/// \returns Nul-terminated string with the name of the alignment (the same as the enum value name)
	/// \sa vertical_align_names
	constexpr const char* to_name(vertical_align v) { return vertical_align_names[int(v)]; }
	/// Get alignment name
	/// \returns Nul-terminated string with the name of the alignment (the same as the enum value name)
	/// \sa align_names
	constexpr const char* to_name(align a) { return align_names[int(a)]; }
	/// Get natural alignment name
	/// \returns Nul-terminated string with the natural name of the alignment (containg spaces)
	/// \sa align_names_natural
	constexpr const char* to_natural_name(align a) { return align_names_natural[int(a)]; }

	namespace detail
	{
		static constexpr inline auto horizontal_align_mask = int(horizontal_align::left) | int(horizontal_align::center) | int(horizontal_align::right);
		static constexpr inline auto vertical_align_mask = int(vertical_align::top) | int(vertical_align::middle) | int(vertical_align::bottom);
	}

	/// Returns \c alignment as if it was on a vertical line
	constexpr vertical_align to_vertical(horizontal_align alignment) { return vertical_align{ (int(alignment) & detail::horizontal_align_mask) << 2 }; }
	/// Returns \c alignment as if it was on a vertical line
	constexpr vertical_align to_vertical(vertical_align alignment) { return alignment; }
	/// Returns \c alignment as if it was on a horizontal line
	constexpr horizontal_align to_horizontal(vertical_align alignment) { return horizontal_align{ (int(alignment) & detail::vertical_align_mask) >> 2 }; }
	/// Returns \c alignment as if it was on a horizontal line
	constexpr horizontal_align to_horizontal(horizontal_align alignment) { return alignment; }

	/// Gets just the vertical component from the \ref align (as a \ref vertical_align)
	constexpr vertical_align vertical_from(align alignment) { return vertical_align{ (int(alignment) & detail::vertical_align_mask) }; }
	/// Gets just the horizontal component from the \ref align (as a \ref horizontal_align)
	constexpr horizontal_align horizontal_from(align alignment) { return horizontal_align{ (int(alignment) & detail::horizontal_align_mask) }; }

	/// Gets a mew \ref align that only has the vertical component
	constexpr align only_vertical(align alignment) { return align{ (int(alignment) & detail::vertical_align_mask) }; }
	/// Gets a mew \ref align that only has the horizontal component
	constexpr align only_horizontal(align alignment) { return align{ (int(alignment) & detail::horizontal_align_mask) }; }

	/// Returns the opposite aligment
	/// \note The opposite of `justify` is `justify`
	constexpr vertical_align to_opposite(vertical_align alignment) { return vertical_align{ ((2 - (int(alignment) >> 2)) & detail::horizontal_align_mask) << 2 }; }

	/// Returns the opposite aligment
	/// \note The opposite of `justify` is `justify`
	constexpr horizontal_align to_opposite(horizontal_align alignment) { return horizontal_align{ (2 - int(alignment)) & detail::horizontal_align_mask }; }

	/// Gets an \ref align that is opposite on bost axes
	/// \note The opposite of `justify` is `justify`
	constexpr align to_opposite(align alignment) { return to_opposite(vertical_from(alignment)) | to_opposite(horizontal_from(alignment)); }

	/// Returns the 2D alignment rotated clockwise.
	///
	/// Imagine the \ref align as a point on a perimeter of a square:
	/// \code
	///     TL-----TC-----TR
	///     |              |
	///     |              |
	///     |              |
	///     ML     MC     MR
	///     |              |
	///     |              |
	///     |              |
	///     BL-----BC-----BR
	/// \endcode
	/// Rotating an \ref align is like rotating the square - so, for example \c top_center becomes \c middle_right
	/// \sa rotated_counter_clockwise
	constexpr align rotated_clockwise(align alignment) { return to_horizontal(to_opposite(vertical_from(alignment))) | to_vertical(horizontal_from(alignment)); }
	
	/// Like \ref rotated_clockwise but counter clockwise
	/// \sa rotated_clockwise
	constexpr align rotated_counter_clockwise(align alignment) { return to_horizontal(vertical_from(alignment)) | to_vertical(to_opposite(horizontal_from(alignment))); }

	constexpr align flipped_horizontally(align alignment) { return vertical_from(alignment) | to_opposite(horizontal_from(alignment)); }
	constexpr align flipped_vertically(align alignment) { return to_opposite(vertical_from(alignment)) | horizontal_from(alignment); }

	template <typename T>
	constexpr T aligned(const T& width, const T& max_width, horizontal_align align) {
		switch (align)
		{
		case horizontal_align::center: return (max_width / 2 - width / 2);
		case horizontal_align::right: return (max_width - width);
		default: return 0;
		}
	}

	template <typename T>
	constexpr T aligned(const T& width, const T& max_width, vertical_align align) {
		switch (align)
		{
		case vertical_align::middle: return (max_width / 2 - width / 2);
		case vertical_align::bottom: return (max_width - width);
		default: return 0;
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

	///@}
}

#ifdef GHPL_HAS_REC2
#include "align+rec2.h"
#endif

/// \var ghassanpl::align ghassanpl::align::top_left
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::top_center
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::top_right
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::middle_left
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::middle_right
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::middle_center
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::bottom_left
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::bottom_center
/// \hideinitializer
/// \var ghassanpl::align ghassanpl::align::bottom_right
/// \hideinitializer