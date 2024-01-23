/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include "span.h"
#include <functional>

#define GHPL_HAS_REC2

/// An extension to the `glm` library. Adds a class template that represents a 2D axis-aligned rectangle.

namespace ghassanpl
{
	template <template<typename> typename HASHER, typename FIRST, typename... T>
	[[nodiscard]] constexpr size_t hash(FIRST&& first, T&&... values);

	static inline constexpr struct bounding_box_for_t {} bounding_box_for;

	template <typename T>
	struct trec2
	{
		glm::tvec2<T> p1 = { 0,0 };
		glm::tvec2<T> p2 = { 0,0 };

		using tvec = glm::tvec2<T>;
		using value_type = T;

		constexpr trec2() noexcept = default;
		constexpr trec2(tvec a, tvec b) noexcept : p1(a), p2(b) { }
		explicit constexpr trec2(span<tvec const> points) noexcept
		{
			*this = trec2::exclusive();
			for (auto& p : points)
				include(p);
		}

		template <GHPL_TYPENAME(std::same_as<tvec>)... ARGS>
		explicit constexpr trec2(bounding_box_for_t, ARGS&&... args) noexcept
		{
			*this = trec2::exclusive();
			(this->include(std::forward<ARGS>(args)), ...);
		}

		constexpr explicit trec2(tvec a) noexcept : p1(), p2(a) { }
		constexpr trec2(T x1, T y1, T x2, T y2) noexcept : p1(x1, y1), p2(x2, y2) { }
		constexpr trec2(const trec2&) noexcept = default;
		constexpr trec2(trec2&&) noexcept = default;
		template <typename U>
		constexpr explicit trec2(const trec2<U>& other) noexcept : p1(glm::tvec2<U>(other.p1)), p2(glm::tvec2<U>(other.p2)) {}
		template <typename U>
		constexpr explicit trec2(trec2<U>&& other) noexcept : p1(glm::tvec2<U>(other.p1)), p2(glm::tvec2<U>(other.p2)) {}

		constexpr trec2& operator=(const trec2&) noexcept = default;
		constexpr trec2& operator=(trec2&&) noexcept = default;

		static constexpr trec2 from_points(span<tvec const> points) noexcept { return trec2{points}; }
		template <GHPL_TYPENAME(std::same_as<tvec>)... ARGS>
		static constexpr trec2 from_points(ARGS&&... args) noexcept { return trec2(bounding_box_for, std::forward<ARGS>(args)...); }
		static constexpr trec2 from_size(tvec s) noexcept { return { tvec{}, s }; };
		static constexpr trec2 from_size(tvec p, tvec s) noexcept { return { p, p + s }; };
		static constexpr trec2 from_size(T x, T y, T w, T h) noexcept { return { x, y, x + w, y + h }; };
		static constexpr trec2 from_center_and_size(tvec p, tvec s) noexcept { return { p - s / T(2), p + s / T(2) }; };
		static constexpr trec2 from_center_and_size(T x, T y, T w, T h) noexcept { return { x - w / T(2), y - h / T(2), x + w / T(2), y + h / T(2) }; };

		static constexpr trec2 invalid() noexcept { return { T{1}, T{1}, T{-1}, T{-1} }; };
		static constexpr trec2 exclusive() noexcept
		{
			if constexpr (std::numeric_limits<T>::has_infinity)
				return { std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity() };
			else
				return { std::numeric_limits<T>::max(), std::numeric_limits<T>::max(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest() };
		}

		static constexpr trec2 inclusive() noexcept
		{
			if constexpr (std::numeric_limits<T>::has_infinity)
				return { -std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity(), std::numeric_limits<T>::infinity() };
			else
				return { std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max() };
		}

		constexpr trec2 operator+(tvec op) const noexcept { return { p1 + op, p2 + op }; }
		constexpr trec2& operator+=(tvec op) noexcept { p1 += op; p2 += op; return *this; };
		constexpr trec2 operator-(tvec op) const noexcept { return { p1 - op, p2 - op }; }
		constexpr trec2& operator-=(tvec op) noexcept { p1 -= op; p2 -= op; return *this; };
		constexpr trec2 operator*(T op) const noexcept { return { p1 * op, p2 * op }; }
		constexpr trec2 operator/(T op) const noexcept { return { p1 / op, p2 / op }; }
		constexpr trec2 operator*(tvec op) const noexcept { return { p1 * op, p2 * op }; }
		constexpr trec2 operator/(tvec op) const noexcept { return { p1 / op, p2 / op }; }

		constexpr auto operator==(trec2 const& other) const noexcept { return p1 == other.p1 && p2 == other.p2; }
		/// TODO: Write this manually (what does it even mean to compare two rectangles?)
		//constexpr auto operator<=>(trec2 const& other) const noexcept = default;

		constexpr tvec operator[](size_t i) const noexcept
		{
			switch (i % 4)
			{
			case 0: return p1;
			case 1: return right_top();
			case 2: return p2;
			case 3: return left_bottom();
			}
			return {}; /// unreachable
		}

#ifdef __cpp_explicit_this_parameter
		template <typename U> struct values_t { U left; U top; U right; U bottom; };
		template <typename U> values_t(U&& left, U&& top, U&& right, U&& bottom) -> values_t<U>;
		template <typename SELF>
		constexpr auto values(this SELF&& self) noexcept
		{
			return values_t {
				std::forward_like<SELF>(self.p1.x),
				std::forward_like<SELF>(self.p1.y),
				std::forward_like<SELF>(self.p2.x),
				std::forward_like<SELF>(self.p2.y)
			};
		}
#endif

		constexpr tvec size() const noexcept { return p2 - p1; }
		constexpr tvec position() const noexcept { return p1; }
		constexpr trec2& set_position(tvec pos) noexcept { p2 += pos - p1; p1 = pos; return *this; }
		constexpr trec2& set_position(T x, T y) noexcept { p2.x += x - p1.x; p2.y += y - p1.y; p1 = { x, y }; return *this; }
		constexpr trec2& set_x(T x) noexcept { p2.x += x - p1.x; p1.x = x; return *this; }
		constexpr trec2& set_y(T y) noexcept { p2.y += y - p1.y; p1.y = y; return *this; }
		constexpr trec2& set_size(tvec size) noexcept { p2 = p1 + size; return *this; }
		constexpr trec2& set_size(T w, T h) noexcept { p2.x = p1.x + w; p2.y = p1.y + h; return *this; }
		
		constexpr trec2& grow(T by) noexcept { p1.x -= by; p1.y -= by; p2.x += by; p2.y += by; return *this; }
		constexpr trec2& shrink(T by) noexcept { return grow(-by); }
		constexpr trec2& grow(tvec by) noexcept { p1 -= by; p2 += by; return *this; }
		constexpr trec2& shrink(tvec by) noexcept { return grow(-by); }
		constexpr trec2 grown(T by) const noexcept { auto copy = *this; copy.p1.x -= by; copy.p1.y -= by; copy.p2.x += by; copy.p2.y += by; return copy; }
		constexpr trec2 shrunk(T by) const noexcept { return grown(-by); }
		constexpr trec2 grown(tvec by) const noexcept { auto copy = *this; copy.p1 -= by; copy.p2 += by; return copy; }
		constexpr trec2 shrunk(tvec by) const noexcept { return grown(-by); }
		constexpr trec2& grow(T left, T top, T right, T bottom) noexcept { p1.x -= left; p1.y -= top; p2.x += right; p2.y += bottom; return *this; }
		constexpr trec2& shrink(T left, T top, T right, T bottom) noexcept { return grow(-left, -top, -right, -bottom); }
		constexpr trec2 grown(T left, T top, T right, T bottom) const noexcept { auto copy = *this; copy.p1.x -= left; copy.p1.y -= top; copy.p2.x += right; copy.p2.y += bottom; return copy; }
		constexpr trec2 shrunk(T left, T top, T right, T bottom) const noexcept { return grown(-left, -top, -right, -bottom); }

		constexpr trec2 at_position(tvec pos) const noexcept { auto copy = *this; copy.set_position(pos); return copy; }
		constexpr trec2 at_position(T x, T y) const noexcept { auto copy = *this; copy.set_position(x, y); return copy; }
		constexpr trec2 sized(tvec size) const noexcept { auto copy = *this; copy.set_size(size); return copy; }
		constexpr trec2 sized(T w, T h) const noexcept { auto copy = *this; copy.set_size(w, h); return copy; }
		constexpr trec2 translated(tvec op) const noexcept { return *this + op; }
		constexpr trec2 translated(T x, T y) const noexcept { return *this + tvec{ x, y }; }
		constexpr trec2 scaled(tvec op) const noexcept { return *this * op; }
		constexpr trec2 scaled(T x, T y) const noexcept { return *this * tvec{ x, y }; }
		constexpr trec2 scaled(T s) const noexcept { return *this * s; }
		constexpr T width() const noexcept { return p2.x - p1.x; };
		constexpr T height() const noexcept { return p2.y - p1.y; };
		constexpr trec2& set_width(T w) noexcept { p2.x = p1.x + w; return *this; };
		constexpr trec2& set_height(T h) noexcept { p2.y = p1.y + h; return *this; };
		constexpr T x() const noexcept { return p1.x; }
		constexpr T y() const noexcept { return p1.y; }
		constexpr T left() const noexcept { return p1.x; }
		constexpr T top() const noexcept { return p1.y; }
		constexpr T right() const noexcept { return p2.x; }
		constexpr T bottom() const noexcept { return p2.y; }
		constexpr tvec left_top() const noexcept { return p1; }
		constexpr tvec left_bottom() const noexcept { return { p1.x, p2.y }; }
		constexpr tvec right_top() const noexcept { return { p2.x, p1.y }; }
		constexpr tvec right_bottom() const noexcept { return p2; }
		constexpr tvec half_size() const noexcept { return (p2 - p1) / T{ 2 }; }
		constexpr tvec center() const noexcept { return p1 + half_size(); }
		constexpr trec2& set_center(tvec pos) noexcept { const auto hw = half_size(); p1 = pos - hw; p2 = pos + hw; return *this; }
		constexpr trec2 at_center(tvec pos) const noexcept { auto copy = *this; copy.set_center(pos); return copy; }

		constexpr trec2 local() const noexcept { return { tvec{}, size() }; }
		constexpr trec2 relative_to(trec2 const& other) const noexcept { return { p1 - other.position(), p2 - other.position() }; }

		constexpr trec2 to_global(trec2 const& parent_rect) const noexcept { return { p1 + parent_rect.position(), p2 + parent_rect.position() }; }

		constexpr glm::vec2 to_rect_space(tvec world_space) const noexcept { return glm::vec2{ world_space - p1 } / glm::vec2{ size() }; }
		constexpr tvec to_world_space(glm::vec2 rect_space) const noexcept { return tvec{ rect_space * glm::vec2{ size() } } + p1; }

		constexpr trec2& include(tvec pt) noexcept { this->p1 = glm::min(this->p1, pt); this->p2 = glm::max(this->p2, pt); return *this; };
		constexpr trec2& include(trec2 const& rec) noexcept { return this->include(rec.p1).include(rec.p2); };

		constexpr trec2 including(tvec pt) const noexcept { return { glm::min(this->p1, pt), glm::max(this->p2, pt) }; };
		constexpr trec2 including(trec2 const& rec) const noexcept { return this->include(rec.p1).include(rec.p2); };

		constexpr bool intersects(trec2 const& other) const noexcept
		{
			return (left() <= other.right() && other.left() <= right() && top() <= other.bottom() && other.top() <= bottom());
		}

		constexpr trec2 intersection(trec2 const& other) const noexcept
		{
			auto x1 = std::max(std::min(p1.x, p2.x), std::min(other.p1.x, other.p2.x));
			auto y1 = std::max(std::min(p1.y, p2.y), std::min(other.p1.y, other.p2.y));
			auto x2 = std::min(std::max(p1.x, p2.x), std::max(other.p1.x, other.p2.x));
			auto y2 = std::min(std::max(p1.y, p2.y), std::max(other.p1.y, other.p2.y));
			return { x1,y1,x2,y2 };
		}

		constexpr trec2 clipped_to(trec2 const& other) const noexcept
		{
			return intersection(other);
		}

		constexpr bool contains(glm::vec<2, T> const& other) const noexcept
		{
			return other.x >= p1.x && other.y >= p1.y && other.x < p2.x && other.y < p2.y;
		}

		/// \pre `other` must be valid
		constexpr bool contains(trec2 const& other) const noexcept
		{
			return other.p1.x >= p1.x && other.p1.y >= p1.y && other.p2.x <= p2.x && other.p2.y <= p2.y;
		}

		constexpr bool is_valid() const noexcept
		{
			return p1.x <= p2.x && p1.y <= p2.y;
		}

		constexpr trec2 valid() const noexcept
		{
			auto copy = *this;
			if (copy.p1.x > copy.p2.x) std::swap(copy.p1.x, copy.p2.x);
			if (copy.p1.y > copy.p2.y) std::swap(copy.p1.y, copy.p2.y);
			return copy;
		}

		constexpr trec2& make_valid() noexcept
		{
			if (p1.x > p2.x) std::swap(p1.x, p2.x);
			if (p1.y > p2.y) std::swap(p1.y, p2.y);
			return *this;
		}

		constexpr std::pair<trec2, trec2> split_vertical(T top_height) const noexcept
		{
			if (top_height < 0)
				top_height = this->height() + top_height;
			return { trec2::from_size(this->p1, {this->width(), top_height}), trec2::from_size(this->p1 + tvec{0, top_height}, {this->width(), this->height() - top_height}) };
		}

		constexpr std::pair<trec2, trec2> split_horizontal(T left_width) const noexcept
		{
			if (left_width < 0)
				left_width = this->width() + left_width;
			return { trec2::from_size(this->p1, {left_width, this->height()}), trec2::from_size(this->p1 + tvec{left_width, 0}, {this->width() - left_width, this->height()})};
		}

		constexpr T calculate_area() const noexcept { return width() * height(); }

		constexpr T edge_length() const noexcept { return (width() + height()) * 2; }

		constexpr glm::vec2 edge_point_alpha(double edge_progress) const
		{
			edge_progress = glm::fract(edge_progress);
			const auto w = width();
			const auto h = height();
			const auto el = (w + h) * 2;
			const auto d = static_cast<T>(edge_progress * el);
			if (d < w)
				return glm::mix(this->left_top(), this->right_top(), d / w);
			else if (d < w + h)
				return glm::mix(this->right_top(), this->right_bottom(), (d - w) / h);
			else if (d < w + h + w)
				return glm::mix(this->right_bottom(), this->left_bottom(), (d - (w + h)) / w);
			else
				return glm::mix(this->left_bottom(), this->left_top(), (d - (w + h + w)) / h);
		}

		constexpr glm::vec2 edge_point(double edge_pos) const
		{
			const auto w = width();
			const auto h = height();
			edge_pos = fmod(edge_pos, (w + h) * 2);
			if (edge_pos < w)
				return glm::mix(this->left_top(), this->right_top(), edge_pos / w);
			else if (edge_pos < w + h)
				return glm::mix(this->right_top(), this->right_bottom(), (edge_pos - w) / h);
			else if (edge_pos < w + h + w)
				return glm::mix(this->right_bottom(), this->left_bottom(), (edge_pos - (w + h)) / w);
			else
				return glm::mix(this->left_bottom(), this->left_top(), (edge_pos - (w + h + w)) / h);
		}

		constexpr trec2 bounding_box() const noexcept
		{
			return *this;
		}

		constexpr glm::vec2 projected(glm::vec2 pt) const
		{
			const auto d = (pt - p1) / size();
			const auto c = glm::clamp(d, glm::vec2{ 0 }, glm::vec2{ 1 });
			return p1 + c * size();
		}

		/// TODO: Distance from point to this
	};


	template <typename T>
	trec2<T> operator+(glm::tvec2<T> op, trec2<T> rec) noexcept { return { rec.p1 + op, rec.p2 + op }; }

	template <typename T, typename STRINGIFIER>
	auto stringify(STRINGIFIER& str, trec2<T>& b) { return str('[', b.p1.x, ',', b.p1.y, ',', b.p2.x, ',', b.p2.y, ']'); }
	template <typename T, typename STRINGIFIER>
	auto stringify(STRINGIFIER& str, trec2<T> const& b) { return str('[', b.p1.x, ',', b.p1.y, ',', b.p2.x, ',', b.p2.y, ']'); }
}


template <typename T>
struct std::hash<ghassanpl::trec2<T>>
{
	std::size_t operator()(ghassanpl::trec2<T> const& s) const noexcept
	{
		return ghassanpl::hash(s.p1, s.p2);
	}
};

#ifdef GHPL_HAS_ALIGN
#include "align+rec2.h"
#endif