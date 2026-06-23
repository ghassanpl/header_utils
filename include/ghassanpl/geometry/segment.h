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
			/// TODO: Claude generated now :P Probably major overkill

			/// glm::cross is 3D-only; this is the 2D perp-dot product (returns a scalar).
			const auto cross2 = [](tvec const& a, tvec const& b) noexcept -> T { return a.x * b.y - a.y * b.x; };

			const auto p = this->start, q = other.start;
			const auto r = this->vec(), s = other.vec();   // end - start for each
			const auto rr = glm::dot(r, r), ss = glm::dot(s, s);

			/// Relative tolerance: a cross product's magnitude scales with the operands' lengths,
			/// so a bare machine epsilon is the wrong threshold for non-unit vectors. We compare
			/// squared magnitudes throughout to avoid sqrt and keep the test scale-invariant.
			constexpr T eps = std::numeric_limits<T>::epsilon();

			/// Degenerate (zero-length) segments can't be reasoned about via cross products of their
			/// direction vectors, so handle point-containment explicitly first.
			const bool this_is_point = rr <= eps;
			const bool other_is_point = ss <= eps;
			if (this_is_point || other_is_point)
			{
				if (this_is_point && other_is_point)        // both are points: meet iff coincident
					return glm::dot(q - p, q - p) <= eps ? std::optional<tvec>{ p } : std::nullopt;

				const auto pt = this_is_point ? p : q;      // the point
				const auto a = this_is_point ? q : p;       // the proper segment's start
				const auto d = this_is_point ? s : r;       // ...and its direction
				const auto dd = this_is_point ? ss : rr;
				const auto ap = pt - a;
				/// On the segment iff collinear (zero perpendicular distance) and projection within [0,1].
				if (cross2(ap, d) * cross2(ap, d) > eps * eps * glm::dot(ap, ap) * dd)
					return std::nullopt;
				const auto tp = glm::dot(ap, d) / dd;
				return (tp >= T(0) && tp <= T(1)) ? std::optional<tvec>{ pt } : std::nullopt;
			}

			const auto qp = q - p;
			const auto rxs = cross2(r, s);

			if (rxs * rxs <= eps * eps * rr * ss)
			{
				const auto qpxr = cross2(qp, r);
				const bool collinear = qpxr * qpxr <= eps * eps * glm::dot(qp, qp) * rr;
				if (!collinear)
					return std::nullopt;                    // parallel but offset -> never meet

				/// Project other's endpoints onto this segment's [0,1] parameter range.
				auto t0 = glm::dot(qp, r) / rr;
				auto t1 = t0 + glm::dot(s, r) / rr;
				if (t0 > t1) { const auto tmp = t0; t0 = t1; t1 = tmp; }

				const auto lo = t0 < T(0) ? T(0) : t0;
				const auto hi = t1 > T(1) ? T(1) : t1;
				if (lo > hi)
					return std::nullopt;                    // collinear but disjoint
				return p + lo * r;                          // start of the overlapping sub-segment
			}

			/// General case: unique intersection of the two lines, accepted only if it lies on both segments.
			const auto t = cross2(qp, s) / rxs;
			const auto u = cross2(qp, r) / rxs;
			if (t >= T(0) && t <= T(1) && u >= T(0) && u <= T(1))
				return p + t * r;
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