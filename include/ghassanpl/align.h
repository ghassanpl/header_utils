#pragma once

namespace ghassanpl
{

	///************************************************************************/
	/// Align
	///************************************************************************/

	enum class horizontal_align
	{
		left = 0,
		center = 1,
		right = 2,
		justify = 3,

		mask = left | center | right
	};

	enum class vertical_align
	{
		top = 0,
		middle = 4,
		bottom = 8,
		justify = 12,

		mask = top | middle | bottom
	};

	enum class align;

	constexpr inline align operator|(horizontal_align first, vertical_align second) { return align(int(first) | int(second)); }
	constexpr inline align operator|(vertical_align first, horizontal_align second) { return align(int(first) | int(second)); }

	enum class align
	{
		left_top = int(horizontal_align::left | vertical_align::top),
		center_top = int(horizontal_align::center | vertical_align::top),
		right_top = int(horizontal_align::right | vertical_align::top),

		left_middle = int(horizontal_align::left | vertical_align::middle),
		center_middle = int(horizontal_align::center | vertical_align::middle),
		right_middle = int(horizontal_align::right | vertical_align::middle),

		left_bottom = int(horizontal_align::left | vertical_align::bottom),
		center_bottom = int(horizontal_align::center | vertical_align::bottom),
		right_bottom = int(horizontal_align::right | vertical_align::bottom),
	};

	constexpr inline align& operator|=(align& first, horizontal_align second) { return first = align(int(first) | int(second)); }
	constexpr inline align& operator|=(align& first, vertical_align second) { return first = align(int(first) | int(second)); }

	constexpr inline const char* horizontal_align_names[] = { "left", "center", "right", "justify_horizontal" };
	constexpr inline const char* vertical_align_names[] = { "top", "middle", "bottom", "justify_vertical", "middle", "", "", "", "bottom", "", "", "", "justify_vertical" };
		
	constexpr inline vertical_align to_vertical(horizontal_align alignment) { return vertical_align{ (int(alignment) & int(horizontal_align::mask)) >> 2 }; }
	constexpr inline vertical_align to_vertical(vertical_align alignment) { return alignment; }
	constexpr inline horizontal_align to_horizontal(vertical_align alignment) { return horizontal_align{ (int(alignment) & int(vertical_align::mask)) << 2 }; }
	constexpr inline horizontal_align to_horizontal(horizontal_align alignment) { return alignment; }
	constexpr inline vertical_align vertical(align alignment) { return vertical_align{ (int(alignment) & int(vertical_align::mask)) }; }
	constexpr inline horizontal_align horizontal(align alignment) { return horizontal_align{ (int(alignment) & int(horizontal_align::mask)) }; }
	constexpr inline align only_vertical(align alignment) { return align{ (int(alignment) & int(vertical_align::mask)) }; }
	constexpr inline align only_horizontal(align alignment) { return align{ (int(alignment) & int(horizontal_align::mask)) }; }

	template <typename T>
	inline constexpr T align_axis(const T& width, const T& max_width, horizontal_align align) {
		switch (align)
		{
		case horizontal_align::center: return (max_width / 2 - width / 2);
		case horizontal_align::right: return (max_width - width);
		default:
		case horizontal_align::left: return 0;
		}
	}

	template <typename T>
	inline constexpr T align_axis(const T& width, const T& max_width, vertical_align align) {
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