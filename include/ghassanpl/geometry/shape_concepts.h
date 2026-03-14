/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include <optional>

namespace ghassanpl::geometry
{
	/// \ingroup Geometry
	/// @{
	
	/// Any contiguous non-empty set of 2D points
	/// \internal TODO: Should we have projected_on_edge? - returns the point ON the edge that is closest to the given point
	template <typename T, typename SHAPE>
	concept shape = requires (SHAPE const& shape, glm::tvec2<T> pt, T t)
	{
		/// `edge_length()` should return the total length of this shape's edge.
		{ shape.edge_length() } -> std::convertible_to<T>;

		/// `edge_point_alpha(t)` should return a point on the shape edge at percentage `t` from an arbitrary start point.
		/// `t` should be normalized between 0 and 1 where 0 is the beginning of the shape edge, and 1 is the end.
		{ shape.edge_point_alpha(t) } -> std::convertible_to<glm::tvec2<T>>;

		/// `edge_point(t)` should return a point on the shape edge at distance `t` from an arbitrary start point.
		/// `t` should be between 0 and L where L is the `edge_length()`
		{ shape.edge_point(t) } -> std::convertible_to<glm::tvec2<T>>;

		/// `bounding_box()` should return an axis-aligned minimum bounding rectangle of this shape.
		{ shape.bounding_box() } -> std::convertible_to<trec2<T>>;

		/// `closest_point_to(pt)` should return the point of the shape that is closest to `pt`.
		{ shape.closest_point_to(pt) } -> std::convertible_to<glm::tvec2<T>>;

		// interval<T> projected_on_axis(glm::tvec2<T> axis) const;
	};

	/// Returns the distance between a shape and a point
	template <typename T, shape<T> S> auto distance(S const& sh, glm::tvec2<T> pt) { return glm::distance(sh.closest_point_to(pt), pt); }
	template <typename T, shape<T> S> auto distance(glm::tvec2<T> pt, S const& sh) { return glm::distance(sh.closest_point_to(pt), pt); }

	template <typename T, shape<T> S> auto distance_squared(S const& sh, glm::tvec2<T> pt) { const auto d = sh.closest_point_to(pt) - pt; return glm::dot(d, d); }
	template <typename T, shape<T> S> auto distance_squared(glm::tvec2<T> pt, S const& sh) { const auto d = sh.closest_point_to(pt) - pt; return glm::dot(d, d); }

	template <typename T>
	std::optional<T> axis_overlaps(shape<T> auto const& a, shape<T> auto const& b, glm::vec2 const& axis)
	{
		interval<T> first = a.projected_on_axis(axis);
		interval<T> second = b.projected_on_axis(axis);

		return first.overlaps(second) ? first.overlap(second) : std::nullopt;
	}

	/// A `shape` for which the concept of an "area" has a finite interpretation
	template <typename T, typename SHAPE>
	concept area_shape = shape<T, SHAPE> && requires (SHAPE const& shape, glm::tvec2<T> pt, T t) {
		{ shape.contains(pt) } -> std::convertible_to<bool>;
		{ shape.calculate_area() } -> std::convertible_to<T>;
		{ shape.centroid() } -> std::convertible_to<glm::tvec2<T>>;
		/*
		{ shape.moment_of_inertia() } -> std::convertible_to<T>;
		{ shape.moment_of_inertia(shape.centroid()) } -> std::convertible_to<T>;
		{ shape.moment_of_inertia(shape.centroid(), shape.centroid()) } -> std::convertible_to<T>;
		*/
	};

	/// A `shape` with vertices and edges.
	/// Not specified in terms of `area_shape` because those operations might be too costly for the specified type to handle
	template <typename T, typename SHAPE>
	concept polygon_shape = shape<T, SHAPE> && requires (SHAPE const& shape) {
		{ shape.for_each_edge([](glm::tvec2<T> const& a, glm::tvec2<T> const& b) {}) };
		{ shape.for_each_vertex([](glm::tvec2<T> const& vertex) {}) };
		{ shape.vertex_count() } -> std::convertible_to<size_t>;
		{ shape.edge_count() } -> std::convertible_to<size_t>;
		{ shape.edge(size_t{}) } -> std::convertible_to<std::optional<std::pair<glm::tvec2<T>, glm::tvec2<T>>>>;
		{ shape.vertex(size_t{}) } -> std::convertible_to<std::optional<glm::tvec2<T>>>;
	};
	
	/// An `area_shape` with vertices and edges.
	template <typename T, typename SHAPE>
	concept polygon_area_shape = area_shape<T, SHAPE> && polygon_shape<T, SHAPE>;

	/// Any type that we can index into which will return a tvec2 of some sort. This includes, for example, std::vector<vec2>.
	template <typename POLY>
	concept indexable_polygonlike = requires (POLY const& polygon, size_t i) {
		/// TODO: I wish we could do something like -> convertible_to_specialization_of<glm::tvec2> but I think that would require additional stuff
		/// like deduction guides for tvecN
		{ polygon[i] } -> is_specialization_of<glm::tvec2>;
	};

	/// TODO: Separating axis overlap test

	/// @}
}