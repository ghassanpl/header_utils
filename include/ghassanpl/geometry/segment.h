/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

namespace ghassanpl::geometry
{

	/// \ingroup Geometry
	/// Represents a 2D segment (technically a `polygon_shape`).
	template <typename TVEC, typename CRTP>
	struct tbasic_segment
	{
		using tvec = TVEC;
		using value_type = typename tvec::value_type;

		tvec start{};
		tvec end{};

		constexpr tbasic_segment() noexcept = default;
		constexpr tbasic_segment(tvec const& start, tvec const& end) noexcept : start(start), end(end) {}
		constexpr tbasic_segment(std::pair<tvec, tvec> const& p) noexcept : start(p.first), end(p.second) {}

		constexpr tvec vec() const noexcept { return end - start; }
		tvec dir() const noexcept { return glm::normalize(vec()); }
		auto length() const noexcept { return glm::distance(start, end); }
		constexpr auto center() const noexcept { return (start + end) / value_type(2); }

		static constexpr CRTP from_offset(tvec const& start, tvec const& offset) noexcept { return { start, start + offset }; }
		static constexpr CRTP from_dir(tvec const& start, tvec const& dir, value_type len) noexcept { return CRTP{ start, start + dir * len }; }
		static constexpr CRTP from_pair(std::pair<tvec, tvec> const& p) noexcept { return { p.first, p.second }; }

		CRTP& set_position(tvec const& pos) noexcept { const auto d = vec(); start = pos; end = pos + d; return reinterpret_cast<CRTP&>(*this); }
		CRTP& operator+=(tvec const& offs) noexcept { start += offs; end += offs; return reinterpret_cast<CRTP&>(*this); }
		CRTP& operator-=(tvec const& offs) noexcept { start -= offs; end -= offs; return reinterpret_cast<CRTP&>(*this); }
		CRTP& translate(tvec const& offs) noexcept { return this->operator+=(offs); }

		CRTP& set_length(value_type len) noexcept { end = start + dir() * len; return reinterpret_cast<CRTP&>(*this); }
		CRTP& set_length_around_center(value_type len) noexcept
		{
			const auto c = center();
			const auto d = dir();
			const auto hlen = len / value_type(2);
			start = c - d * hlen;
			end = c + d * hlen;
			return reinterpret_cast<CRTP&>(*this);
		}

		CRTP& grow(value_type len) noexcept { const auto d = dir(); start -= d * len; end += d * len; return reinterpret_cast<CRTP&>(*this); }
		CRTP& shrink(value_type len) noexcept { const auto d = dir(); start += d * len; end -= d * len; return reinterpret_cast<CRTP&>(*this); }

		/// Shape interface

		value_type edge_length() const { return length(); }
		tvec edge_point_alpha(std::floating_point auto t) const { return glm::mix(start, end, t); }
		tvec edge_point(std::floating_point auto t) const { return glm::mix(start, end, t / edge_length()); }

		/// Polygon shape interface

		template <typename FUNC>
		void for_each_edge(FUNC&& func) const
		{
			func(start, end);
		}

		template <typename FUNC>
		void for_each_vertex(FUNC&& func) const
		{
			func(start);
			func(end);
		}

		size_t vertex_count() const { return 2; }

		size_t edge_count() const { return 1; }

		auto edge(size_t index) const -> std::optional<std::pair<tvec, tvec>>
		{
			if (index == 0)
				return std::make_pair(start, end);
			return std::nullopt;
		}

		auto vertex(size_t index) const -> std::optional<tvec>
		{
			if (index == 0)
				return start;
			if (index == 1)
				return end;
			return std::nullopt;
		}
	};


	/// \ingroup Geometry
	/// Represents a 2D segment (technically a `polygon_shape`).
	template <std::floating_point T>
	struct tsegment : public tbasic_segment<glm::tvec2<T>, tsegment<T>>
	{
		using parent_type = tbasic_segment<glm::tvec2<T>, tsegment<T>>;
		using parent_type::parent_type;
		using typename parent_type::tvec;
		using typename parent_type::value_type;

		basic_line_t<T> line() const noexcept { return line_crossing_points(this->start, this->end); }

		constexpr std::optional<tvec> intersection(tsegment const& other) const noexcept
		{
			/// TODO: Copilot-generated :P Needs testing
			const auto r = this->vec();
			const auto s = other.vec();
			const auto rxs = glm::cross(r, s);
			const auto qp = other.start - this->start;
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
				return this->start + t * r;
			return std::nullopt;
		}

		/// Shape interface

		trec2<T> bounding_box() const { return trec2<T>::from_points({ &this->start, &this->start + 2 }); }

		tvec closest_point_to(tvec pt) const
		{
			const auto dir = this->vec();
			const auto d1 = glm::dot(pt - this->start, dir);
			if (d1 <= 0)
				return this->start;
			const auto d2 = glm::dot(dir, dir);
			if (d1 > d2)
				return this->end;
			return this->start + dir * (d1 / d2);
		}

		interval<T> projected_on_axis(tvec const& axis) const
		{
			const auto p0 = glm::dot(axis, this->start);
			const auto p1 = glm::dot(axis, this->end);
			return { std::min(p0, p1), std::max(p0, p1) };
		}
	};

	using segment = tsegment<float>;

	/// \ingroup Geometry
	/// Represents a 2D segment (technically a `polygon_shape`).
	template <std::floating_point T>
	struct tsegment3d : public tbasic_segment<glm::tvec3<T>, tsegment3d<T>>
	{
		using parent_type = tbasic_segment<glm::tvec3<T>, tsegment3d<T>>;
		using parent_type::parent_type;
		using typename parent_type::tvec;
		using parent_type::value_type;

		tsegment3d(tsegment<T> seg, float z) : tsegment3d(glm::tvec3<T>(seg.start, z), glm::tvec3<T>(seg.end, z)) { }
	};

	using segment3d = tsegment3d<float>;
}

#define GHPL_GEOMETRY_SEGMENT 1
#ifdef GHPL_GEOMETRY_SEGMENT
#include "ray+segment.h"
#endif