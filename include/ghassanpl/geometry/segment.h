/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

namespace ghassanpl::geometry
{

	template <std::floating_point T>
	struct tsegment
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;

		tvec start{};
		tvec end{};

		tvec vec() const noexcept { return end - start; }
		tvec dir() const noexcept { return glm::normalize(vec()); }
		auto length() const noexcept { return glm::distance(start, end); }
		auto center() const noexcept { return (start + end) / T(2); }

		static tsegment from_offset(tvec const& start, tvec const& offset) noexcept { return { start, start + offset }; }
		static tsegment from_dir(tvec const& start, tvec const& dir, T len) noexcept { return { start, start + dir * len }; }

		basic_line_t<T> line() const noexcept { return line_crossing_points(start, end); }

		tsegment& set_position(tvec const& pos) noexcept { const auto d = vec(); start = pos; end = pos + d; return *this; }
		tsegment& operator+=(tvec const& offs) noexcept { start += offs; end += offs; return *this; }
		tsegment& operator-=(tvec const& offs) noexcept { start -= offs; end -= offs; return *this; }
		tsegment& translate(tvec const& offs) noexcept { return this->operator+=(offs); }

		tsegment& set_length(T len) noexcept { end = start + dir() * len; return *this; }
		tsegment& set_length_around_center(T len) noexcept
		{
			const auto c = center();
			const auto d = dir();
			const auto hlen = len / T(2);
			start = c - d * hlen;
			end = c + d * hlen;
			return *this;
		}

		tsegment& grow(T len) noexcept { const auto d = dir(); start -= d * len; end += d * len; return *this; }
		tsegment& shrink(T len) noexcept { const auto d = dir(); start += d * len; end -= d * len; return *this; }

		constexpr std::optional<tvec> intersection(tsegment const& other) const noexcept
		{
			/// TODO: Copilot-generated :P Needs testing
			const auto r = vec();
			const auto s = other.vec();
			const auto rxs = glm::cross(r, s);
			const auto qp = other.start - start;
			const auto qpxr = glm::cross(qp, r);
			if (glm::abs(rxs) < std::numeric_limits<T>::epsilon() && glm::abs(qpxr) < std::numeric_limits<T>::epsilon())
			{
				/// Colinear
				const auto t0 = glm::dot(qp, r) / glm::dot(r, r);
				const auto t1 = t0 + glm::dot(s, r) / glm::dot(r, r);
				if ((t0 >= 0 && t0 <= 1) || (t1 >= 0 && t1 <= 1))
					return std::nullopt;
				return std::nullopt;
			}
			if (glm::abs(rxs) < std::numeric_limits<T>::epsilon() && glm::abs(qpxr) > std::numeric_limits<T>::epsilon())
				return std::nullopt;
			const auto t = glm::cross(qp, s) / rxs;
			const auto u = glm::cross(qp, r) / rxs;
			if (rxs != 0 && t >= 0 && t <= 1 && u >= 0 && u <= 1)
				return start + t * r;
			return std::nullopt;
		}

		/// Shape interface

		T edge_length() const { return length(); }
		tvec edge_point_alpha(T t) const { return glm::mix(start, end, t); }
		tvec edge_point(T t) const { return glm::mix(start, end, t / edge_length()); }
		trec2<T> bounding_box() const { return trec2<T>::from_points({ &start, &start + 2 }); }
		tvec projected(tvec pt) const
		{
			const auto dir = this->vec();
			const auto d1 = glm::dot(pt - start, dir);
			if (d1 <= 0)
				return start;
			const auto d2 = glm::dot(dir, dir);
			if (d1 > d2)
				return end;
			return start + dir * (d1 / d2);
		}
	};

	using segment = tsegment<float>;

	static_assert(shape<segment, float>);
}