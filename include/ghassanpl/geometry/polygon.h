/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "./segment.h"
#include "./triangles.h"
#include <array>

namespace ghassanpl::geometry
{
	template <std::floating_point T>
	struct tpolygon
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;

		std::vector<glm::tvec2<T>> vertices;

		tvec& operator[](size_t index) { return vertices.at(index); }
		tvec const& operator[](size_t index) const { return vertices.at(index); }

		auto cbegin() const { return std::ranges::cbegin(vertices); }
		auto cend() const { return std::ranges::cend(vertices); }
		auto begin() { return std::ranges::begin(vertices); }
		auto end() { return std::ranges::end(vertices); }
		auto begin() const { return std::ranges::cbegin(vertices); }
		auto end() const { return std::ranges::cend(vertices); }
		auto size() const { return std::ranges::size(vertices); }

		bool is_valid() const noexcept { return vertices.size() > 2; }

		struct convexity {
			bool simple = false;
			bool convex = false;
			constexpr bool is_simple() const { return simple; }
			constexpr bool intersects_itself() const { return !simple; }
			constexpr bool is_convex() const { return convex; }
			constexpr bool is_concave() const noexcept { return simple && !convex; }
		};

		convexity calculate_convexity() const noexcept;
		tpolygon<T> convex_hull() const noexcept;

		std::optional<tsegment<T>> edge(size_t index) const
		{
			const auto c = vertices.size();
			if (c < 2 || index >= c - 1) return std::nullopt;
			return tsegment<T>{vertices[index], vertices[index + 1]};
		}

		std::optional<tvec> vertex(size_t index) const
		{
			const auto c = vertices.size();
			if (c < 2 || index >= c) return std::nullopt;
			return vertices[index];
		}

		template <typename FUNC>
		void for_each_edge(FUNC&& func)
		{
			const auto c = vertices.size();
			if (c < 2) return;

			for (size_t i = 0; i < c - 1; ++i)
				func(tsegment<T>{vertices[i], vertices[i + 1]});
		}

		std::vector<tsegment<T>> edges() const noexcept
		{
			std::vector<tsegment<T>> result;
			for_each_edge([&](auto&& seg) { result.push_back(seg); });
			return result;
		}
		
		std::vector<T> interior_angles() const noexcept;

		T edge_length() const
		{
			T result{};
			for_each_edge([&](tvec const& v1, tvec const& v2) {
				result += glm::distance(v1, v2);
			});
			return result;
		}

		tvec edge_point(T t) const
		{
			const auto el = edge_length();
			return edge_point(t, el);
		}

		tvec edge_point_alpha(T t) const
		{
			const auto el = edge_length();
			return edge_point(t * el, el);
		}

		tvec projected(tvec pt) const;

		T calculate_area() const;

		trec2<T> bounding_box() const
		{
			trec2<T> res = trec2<T>::exclusive();
			for (auto& v : vertices)
				res.include(v);
			return res;
		}

		/// http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
		bool contains(tvec test) const
		{
			bool ret = false;
			const size_t nvert = vertices.size();
			for (size_t i = 0, j = nvert - 1; i < nvert; j = i++)
			{
				if (((vertices[i].y > test.y) != (vertices[j].y > test.y)) &&
					(test.x < (vertices[j].x - vertices[i].x) * (test.y - vertices[i].y) / (vertices[j].y - vertices[i].y) + vertices[i].x))
				{
					ret = !ret;
				}
			}
			return ret;
		}

	private:

		tvec edge_point(T t, T precalced_length) const
		{
			auto c = vertices.size();
			if (c < 2) 
				return c ? vertices[0] : tvec{};

			t = fmod(t, precalced_length);
			for (size_t i = 0; i < c - 1; ++i)
			{
				const auto d = glm::distance(vertices[i], vertices[i + 1]);
				if (t <= d)
					return glm::lerp(vertices[i], vertices[i + 1], t / d);

				t -= d;
			}
			return vertices[0];
		}
	};

	using polygon = tpolygon<float>;
	static_assert(shape<polygon, float>);
	static_assert(std::ranges::random_access_range<polygon>);

	template <std::floating_point T, std::integral IDX = size_t>
	struct polygon_triangulation
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;

		tpolygon<T> const& poly;
		std::vector<tindexed_triangle<IDX>> triangles;

		template <typename FUNC>
		void for_each_triangle(FUNC&& func)
		{
			for (auto& tr : triangles)
			{
				return tr.as_triangle(poly);
			}
		}

		T edge_length() const { return poly.edge_length(); }
		tvec edge_point_alpha(T t) const { return poly.edge_point_alpha(t); }
		tvec edge_point(T t) const { return poly.edge_point(t); }
		trec2<T> bounding_box() const { return poly.bounding_box(); }
		tvec projected(glm::tvec2<T> pt) const { return poly.projected(pt); }

		bool contains(glm::tvec2<T> pt) const;
		T calculate_area() const;
	};

	static_assert(area_shape<polygon_triangulation<float>, float>);

	template <std::floating_point T, std::integral IDX = size_t>
	struct triangulated_polygon
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;

		tpolygon<T> poly;
		std::vector<tindexed_triangle<IDX>> triangles;

		T edge_length() const { return poly.edge_length(); }
		tvec edge_point_alpha(T t) const { return poly.edge_point_alpha(t); }
		tvec edge_point(T t) const { return poly.edge_point(t); }
		trec2<T> bounding_box() const { return poly.bounding_box(); }
		tvec projected(glm::tvec2<T> pt) const { return poly.projected(pt); }

		bool contains(glm::tvec2<T> pt) const;
		T calculate_area() const;
	};

	static_assert(area_shape<triangulated_polygon<float>, float>);

	template <typename POLY>
	auto calculate_indexed_triangle_area(POLY const& poly, indexed_triangle const& triangle)
	{
		const auto a = poly[triangle.indices[0]];
		const auto b = poly[triangle.indices[1]];
		const auto c = poly[triangle.indices[2]];
		using T = typename POLY::value_type;
		return geometry::ttriangle<T>{a, b, c}.calculate_area();
	}

	template <typename TR>
	auto calculate_total_area(TR const& trpoly)
	{
		using T = decltype(trpoly.poly)::value_type;
		T result{};
		for (auto& tr : trpoly.triangles)
			result += calculate_indexed_triangle_area(trpoly.poly, tr);
		return result;
	}

	template <typename T>
	polygon_triangulation<T> triangulate(tpolygon<T> const& poly)
	{
		polygon_triangulation<T> result{poly};
		/// https://github.com/mapbox/earcut.hpp
		throw "unimplemented";
		return result;
	}

	namespace immutable
	{
		template <std::floating_point T>
		struct tpolygon
		{
			using tvec = glm::tvec2<T>;
			using value_type = T;
			using mutable_polygon = geometry::tpolygon<T>;

			tpolygon() noexcept = default;
			tpolygon(tpolygon const&) noexcept = default;
			tpolygon(tpolygon&&) noexcept = default;
			tpolygon& operator=(tpolygon const&) noexcept = default;
			tpolygon& operator=(tpolygon&&) noexcept = default;

			template <typename... ARGS>
			requires std::constructible_from<mutable_polygon, ARGS...>
			tpolygon(ARGS&&... args) noexcept
				: m_poly(std::forward<ARGS>(args)...)
			{
			}

			/// Deserialization constructor
			template <typename POLY_OR_ARR>
			tpolygon(POLY_OR_ARR&& poly, std::vector<indexed_triangle> triangles, std::vector<T> cached_triangle_areas, T cached_area)
				: m_poly(std::forward<POLY_OR_ARR>(poly))
				, m_triangles(std::move(triangles))
				, m_cached_triangle_areas(std::move(cached_triangle_areas))
				, m_cached_area(cached_area)
			{

			}


			tvec const& operator[](size_t index) const { return m_poly.vertices.at(index); }

			auto cbegin() const noexcept { return std::ranges::cbegin(m_poly.vertices); }
			auto cend() const noexcept { return std::ranges::cend(m_poly.vertices); }
			auto begin() const noexcept { return std::ranges::cbegin(m_poly.vertices); }
			auto end() const noexcept { return std::ranges::cend(m_poly.vertices); }
			auto size() const noexcept { return std::ranges::size(m_poly.vertices); }

			const auto* operator->() const noexcept { return &m_poly; }

			const auto& polygon() const noexcept { return m_poly; }

			T edge_length() const { return m_poly.edge_length(); }
			tvec edge_point_alpha(T t) const { return m_poly.edge_point_alpha(t); }
			tvec edge_point(T t) const { return m_poly.edge_point(t); }
			trec2<T> bounding_box() const { return m_poly.bounding_box(); }
			tvec projected(glm::tvec2<T> pt) const { return m_poly.projected(pt); }

			bool contains(glm::tvec2<T> pt) const;
			
			T calculate_area() const
			{
				triangulate();
				return m_cached_area;
			}

			auto const& triangles() const
			{
				triangulate();
				return m_triangles;
			}

			auto const& areas() const
			{
				triangulate();
				return m_cached_triangle_areas;
			}

			bool has_triangle(size_t i) const { triangulate(); return i < m_triangles.size(); }
			auto triangle(size_t i) const { triangulate(); return m_triangles.at(i).as_triangle(m_poly); }
			T triangle_area(size_t i) const { triangulate(); return m_cached_triangle_areas.at(i); }

		protected:
			
			geometry::tpolygon<T> m_poly;
			mutable std::vector<indexed_triangle> m_triangles;
			mutable std::vector<T> m_cached_triangle_areas;
			mutable T m_cached_area = std::numeric_limits<T>::lowest();

			void triangulate() const
			{
				if (m_triangles.empty() && m_poly.vertices.size() > 2)
				{
					auto&& [poly, triangles] = geometry::triangulate(m_poly);
					m_triangles = std::move(triangles);

					m_cached_triangle_areas.reserve(m_triangles.size());
					T cached_area{};
					for (auto& tr : m_triangles)
					{
						const auto area = calculate_indexed_triangle_area(poly, tr);
						m_cached_triangle_areas.push_back(area);
						cached_area += area;
					}
					m_cached_area = cached_area;
				}
			}
		};

		using polygon = tpolygon<float>;
		static_assert(area_shape<polygon, float>);
	}

	/// Polyline

	template <std::floating_point T>
	struct tpolyline;
}