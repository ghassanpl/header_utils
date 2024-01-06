/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

namespace ghassanpl::geometry
{
	/// Shape Concept
		
	/*
	T edge_length() const;
	glm::tvec2<T> edge_point_alpha(T t) const;
	glm::tvec2<T> edge_point(T t) const;
	trec2<T> bounding_box() const;
	glm::tvec2<T> projected(glm::tvec2<T> pt) const;
	*/


	template <typename SHAPE, typename T>
	concept shape = requires (SHAPE const& shape, glm::tvec2<T> pt, T t) {
		{ shape.edge_length() } -> std::convertible_to<T>;
		{ shape.edge_point_alpha(t) } -> std::convertible_to<glm::tvec2<T>>;
		{ shape.edge_point(t) } -> std::convertible_to<glm::tvec2<T>>;
		{ shape.bounding_box() } -> std::convertible_to<trec2<T>>;

		/// Returns the point IN the shape that is closest to the given point
		{ shape.projected(pt) } -> std::convertible_to<glm::tvec2<T>>;

		/// TODO: Should we have projected_on_edge? - returns the point ON the edge that is closest to the given point

		/// TODO: { shape.distance_to(pt) } -> std::convertible_to<T>; /// Can be glm::distance(shape.projected(pt), pt)
	};

	template <typename T, shape<T> S> auto distance(S const& sh, glm::tvec2<T> pt) { return glm::distance(sh.projected(pt), pt); }
	template <typename T, shape<T> S> auto distance(glm::tvec2<T> pt, S const& sh) { return glm::distance(sh.projected(pt), pt); }

	template <typename T, shape<T> S> auto distance_squared(S const& sh, glm::tvec2<T> pt) { const auto d = sh.projected(pt) - pt; return glm::dot(d, d); }
	template <typename T, shape<T> S> auto distance_squared(glm::tvec2<T> pt, S const& sh) { const auto d = sh.projected(pt) - pt; return glm::dot(d, d); }

	/// Area Shape Concept
	
	/*
	bool contains(glm::tvec2<T> pt) const;
	T calculate_area() const;
	*/

	template <typename SHAPE, typename T>
	concept area_shape = shape<SHAPE, T> && requires (SHAPE const& shape, glm::tvec2<T> pt, T t) {
		{ shape.contains(pt) } -> std::convertible_to<bool>;
		{ shape.calculate_area() } -> std::convertible_to<T>;
	};

}