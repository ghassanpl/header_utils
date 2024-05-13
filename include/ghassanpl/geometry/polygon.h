/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "./segment.h"
#include "./triangles.h"
#include "shape_concepts.h"
#include <array>

namespace ghassanpl::geometry
{
	/*
	enum class polygon_class
	{
		simple_convex,
		simple_concave,
		complex,
	};
	*/
	struct polygon_classification {
		bool simple = false;
		bool convex = false;
		winding_order winding{};
		constexpr bool is_simple() const { return simple; }
		constexpr bool intersects_itself() const { return !simple; }
		constexpr bool is_convex() const { return convex; }
		constexpr bool is_concave() const noexcept { return simple && !convex; }
	};

	template <std::floating_point T>
	struct tpolygon
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;

		std::vector<glm::tvec2<T>> vertices;

		template <typename... ARGS>
		static tpolygon from_vertices(ARGS&&... args) {
			return tpolygon{ { std::forward<ARGS>(args)... } };
		}

		template <typename S>
		requires polygon_shape<T, S>
		static tpolygon from_shape(S const& shape)
		{
			tpolygon result;
			shape.for_each_vertex([&result](auto const& v) {
				result.vertices.push_back(v);
			});
			return result;
		}

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

		/*
		polygon_classification classify() const noexcept
		{
			// Returns 0 if the polygon is complex (has intersecting edges). 
			// Returns +-1 if it is simple and convex. Returns +-2 if it is simple and concave. 
			// The sign of the returned value indicates whether the polygon is CCW (C) or CW (�
			Int i, ii, j, jj, np, schg = 0, wind = 0;
			//Initialize sign change and winding number.
			Doub p0, p1, d0, d1, pp0, pp1, dd0, dd1, t, tp, t1, t2, crs, crsp = 0.0;
			np = vt.size();
			p0 = vt[0].x[0] - vt[np - 1].x[0];
			p1 = vt[0].x[1] - vt[np - 1].x[1];
			for (i = 0, ii = 1; i < np; i++, ii++) {
				//Loop over edges.
				if (ii == np) ii = 0;
				d0 = vt[ii].x[0] - vt[i].x[0];
				d1 = vt[ii].x[1] - vt[i].x[1];
				crs = p0 * d1 - p1 * d0;
				//Cross product at this vertex.
				if (crs * crsp < 0) schg = 1;
				//Sign change(i.e., concavity) found.
				if (p1 <= 0.0) {
					//Winding number logic as in polywind.
					if (d1 > 0.0 && crs > 0.0) wind++;
				}
				else {
					if (d1 <= 0.0 && crs < 0.0) wind--;
				}
				p0 = d0;
				p1 = d1;
				if (crs != 0.0) crsp = crs;
				//Save previous cross product only if it has a sign!
			}
			if (abs(wind) != 1) return 0;
			//Can already conclude polygon is complex.
			if (schg == 0) return (wind > 0 ? 1 : -1); //Polygon is simple and convex.
			//Drat, we’ve exhausted all the quick tricks and now have to check all pairs of edges for
			//intersections:
			for (i = 0, ii = 1; i < np; i++, ii++) {
				if (ii == np) ii = 0;
				d0 = vt[ii].x[0];
				d1 = vt[ii].x[1];
				p0 = vt[i].x[0];
				p1 = vt[i].x[1];
				tp = 0.0;
				for (j = i + 1, jj = i + 2; j < np; j++, jj++) {
					if (jj == np) { if (i == 0) break; jj = 0; }
					dd0 = vt[jj].x[0];
					dd1 = vt[jj].x[1];
					t = (dd0 - d0) * (p1 - d1) - (dd1 - d1) * (p0 - d0);
					if (t * tp <= 0.0 && j > i + 1) {
						//First loop is only to compute starting tp, hence test on j.
						pp0 = vt[j].x[0];
						pp1 = vt[j].x[1];
						t1 = (p0 - dd0) * (pp1 - dd1) - (p1 - dd1) * (pp0 - dd0);
						t2 = (d0 - dd0) * (pp1 - dd1) - (d1 - dd1) * (pp0 - dd0);
						if (t1 * t2 <= 0.0) return 0;
						// Found an intersection, so done.
					}
					tp = t;
				}
			}
			return (wind > 0 ? 2 : -2);
		}
		*/
		tpolygon<T> convex_hull() const noexcept;

		size_t vertex_count() const { return vertices.size(); }
		size_t edge_count() const { return vertices.size() < 2 ? 0 : vertices.size() - 1; }

		std::optional<std::pair<tvec, tvec>> edge(size_t index) const
		{
			const auto c = vertices.size();
			if (c < 2 || index >= c - 1) return std::nullopt;
			return std::pair{ vertices[index], vertices[index + 1] };
		}

		std::optional<tvec> vertex(size_t index) const
		{
			const auto c = vertices.size();
			if (c < 2 || index >= c) return std::nullopt;
			return vertices[index];
		}

		template <typename FUNC>
		void for_each_vertex(FUNC&& func) const
		{
			for (auto& v : vertices)
				func(v);
		}

		template <typename FUNC>
		void for_each_edge(FUNC&& func) const
		{
			const auto c = vertices.size();
			if (c < 2) return;

			if constexpr (std::invocable<FUNC, tvec const&, tvec const&>)
			{
				for (size_t i = 0; i < c - 1; ++i)
					func(vertices[i], vertices[i + 1]);
			}
			else
			{
				for (size_t i = 0; i < c - 1; ++i)
					func(std::pair{ vertices[i], vertices[i + 1] });
			}
		}

		std::vector<tsegment<T>> edges() const noexcept
		{
			return resulting([this](std::vector<tsegment<T>>& result) {
				for_each_edge(op::push_back_to(result));
			});
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

		tvec closest_point_to(tvec pt) const;

		T calculate_area() const
		{
			T result{};
			for_each_edge([&](tvec const& v1, tvec const& v2) {
				result += v1.x * v2.y - v2.x * v1.y;
			});
			return result * T(0.5);
		}

		tvec centroid() const
		{
			T A = 0;
			T Cx = 0;
			T Cy = 0;
			for (size_t i = 0; i < vertices.size(); ++i)
			{
				const auto xi = vertices[i].x;
				const auto yi = vertices[i].y;
				const auto xi1 = vertices[(i + 1) % vertices.size()].x;
				const auto yi1 = vertices[(i + 1) % vertices.size()].y;
				const auto d = xi * yi1 - xi1 * yi;
				A += d;
				Cx += (xi + xi1) * d;
				Cy += (yi + yi1) * d;
			}
			A *= 3;
			return { Cx / A, Cy / A };
		}

		trec2<T> bounding_box() const
		{
			trec2<T> res = trec2<T>::exclusive();
			for (auto& v : vertices)
				res.include(v);
			return res;
		}

		/// Assumes a simple polygon
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
					return glm::mix(vertices[i], vertices[i + 1], t / d);

				t -= d;
			}
			return vertices[0];
		}
	};

	using polygon = tpolygon<float>;
	static_assert(polygon_shape<float, polygon>);
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
		tvec closest_point_to(glm::tvec2<T> pt) const { return poly.closest_point_to(pt); }

		/// area_shape concept
		bool contains(glm::tvec2<T> pt) const;
		T calculate_area() const;
		tvec centroid() const;
	};

	static_assert(area_shape<float, polygon_triangulation<float>>);

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
		tvec closest_point_to(glm::tvec2<T> pt) const { return poly.closest_point_to(pt); }

		/// area_shape concept
		bool contains(glm::tvec2<T> pt) const;
		T calculate_area() const;
		tvec centroid() const;
	};

	static_assert(area_shape<float, triangulated_polygon<float>>);

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

	/*
	public static void Triangulate(IList<Vector2> points, List<int> populate)
	{
		IList<Vector2> points2 = points;
		if (points2.Count < 3)
		{
			return;
		}

		Span<int> span = ((points2.Count >= 1000) ? ((Span<int>)new int[points2.Count]) : stackalloc int[points2.Count]);
		Span<int> list2 = span;
		if (Area() > 0f)
		{
			for (int i = 0; i < points2.Count; i++)
			{
				list2[i] = i;
			}
		}
		else
		{
			for (int j = 0; j < points2.Count; j++)
			{
				list2[j] = points2.Count - 1 - j;
			}
		}

		int num = points2.Count;
		int num2 = 2 * num;
		int num3 = num - 1;
		while (num > 2)
		{
			if (num2-- <= 0)
			{
				return;
			}

			int num4 = num3;
			if (num <= num4)
			{
				num4 = 0;
			}

			num3 = num4 + 1;
			if (num <= num3)
			{
				num3 = 0;
			}

			int num5 = num3 + 1;
			if (num <= num5)
			{
				num5 = 0;
			}

			if (Snip(num4, num3, num5, num, list2))
			{
				populate.Add(list2[num4]);
				populate.Add(list2[num3]);
				populate.Add(list2[num5]);
				int num6 = num3;
				for (int k = num3 + 1; k < num; k++)
				{
					list2[num6] = list2[k];
					num6++;
				}

				num--;
				num2 = 2 * num;
			}
		}

		populate.Reverse();
		float Area()
		{
			float num7 = 0f;
			int index = points2.Count - 1;
			int num8 = 0;
			while (num8 < points2.Count)
			{
				Vector2 vector = points2[index];
				Vector2 vector2 = points2[num8];
				num7 += vector.X * vector2.Y - vector2.X * vector.Y;
				index = num8++;
			}

			return num7 * 0.5f;
		}

		bool Snip(int u, int v, int w, int n, Span<int> list)
		{
			Vector2 a = points2[list[u]];
			Vector2 b = points2[list[v]];
			Vector2 c = points2[list[w]];
			if (float.Epsilon > (b.X - a.X) * (c.Y - a.Y) - (b.Y - a.Y) * (c.X - a.X))
			{
				return false;
			}

			for (int l = 0; l < n; l++)
			{
				if (l != u && l != v && l != w && InsideTriangle(a, b, c, points2[list[l]]))
				{
					return false;
				}
			}

			return true;
		}
	}
	*/
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
			tvec closest_point_to(glm::tvec2<T> pt) const { return m_poly.closest_point_to(pt); }

			/// area_shape concept
			bool contains(glm::tvec2<T> pt) const;
			
			T calculate_area() const
			{
				triangulate();
				return m_cached_area;
			}

			tvec centroid() const
			{
				triangulate();
				return m_poly.centroid();
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
			mutable tvec m_cached_centroid;
			mutable polygon_classification m_cached_classification;

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
		static_assert(area_shape<float, polygon>);
	}

	/// Polyline

	template <std::floating_point T>
	struct tpolyline;
}