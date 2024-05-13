/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include <optional>

namespace ghassanpl::geometry
{
	/// Shape Concept
		
	/*
	T edge_length() const;
	glm::tvec2<T> edge_point_alpha(T t) const;
	glm::tvec2<T> edge_point(T t) const;
	trec2<T> bounding_box() const;
	glm::tvec2<T> closest_point_to(glm::tvec2<T> pt) const;
	interval<T> projected_on_axis(glm::tvec2<T> axis) const;
	*/


	template <typename T, typename SHAPE>
	concept shape = requires (SHAPE const& shape, glm::tvec2<T> pt, T t) {
		{ shape.edge_length() } -> std::convertible_to<T>;
		{ shape.edge_point_alpha(t) } -> std::convertible_to<glm::tvec2<T>>;
		{ shape.edge_point(t) } -> std::convertible_to<glm::tvec2<T>>;
		{ shape.bounding_box() } -> std::convertible_to<trec2<T>>;

		/// Returns the point IN the shape that is closest to the given point
		{ shape.closest_point_to(pt) } -> std::convertible_to<glm::tvec2<T>>;

		/// TODO: Should we have projected_on_edge? - returns the point ON the edge that is closest to the given point

		/// TODO: { shape.distance_to(pt) } -> std::convertible_to<T>; /// Can be glm::distance(shape.closest_point_to(pt), pt)
	};

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

	/// Area Shape Concept
	
	/*
	bool contains(glm::tvec2<T> pt) const;
	T calculate_area() const;
	*/

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

	/// Polygon Shape Concept
	
	/*
	* void for_each_edge(FUNC&& func) const;
	* void for_each_vertex(FUNC&& func) const;
	* size_t vertex_count() const;
	* size_t edge_count() const;
	* auto edge(size_t index) const -> std::optional<std::pair<glm::tvec2<T>, glm::tvec2<T>>>;
	* auto vertex(size_t index) const -> std::optional<tvec>;
	*/

	template <typename T, typename SHAPE>
	concept polygon_shape = shape<T, SHAPE> && requires (SHAPE const& shape) {
		{ shape.for_each_edge([](glm::tvec2<T> const& a, glm::tvec2<T> const& b) {}) };
		{ shape.for_each_vertex([](glm::tvec2<T> const& vertex) {}) };
		{ shape.vertex_count() } -> std::convertible_to<size_t>;
		{ shape.edge_count() } -> std::convertible_to<size_t>;
		{ shape.edge(size_t{}) } -> std::convertible_to<std::optional<std::pair<glm::tvec2<T>, glm::tvec2<T>>>>;
		{ shape.vertex(size_t{}) } -> std::convertible_to<std::optional<glm::tvec2<T>>>;
	};

	/// TODO: Separating axis overlap test
}