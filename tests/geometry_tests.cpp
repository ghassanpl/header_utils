/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

//#include "../include/ghassanpl/geometry/angles.h"
#include "../include/ghassanpl/geometry/square_grid.h"
#include "../include/ghassanpl/geometry/ellipse.h"
#include "../include/ghassanpl/geometry/polygon.h"
#include "../include/ghassanpl/geometry/segment.h"
#include "../include/ghassanpl/geometry/circle.h"
#include "../include/ghassanpl/geometry/capsule.h"
#include "../include/ghassanpl/geometry/ray.h"

#include "../include/ghassanpl/geometry/points.h"
#include "../include/ghassanpl/geometry/rectangles.h"
#include "../include/ghassanpl/functional.h"
//#include "../include/ghassanpl/geometry/squares.h"
//#include "../include/ghassanpl/geometry/grid_algorithms.h"
//#include "../include/ghassanpl/geometry/block_navigation_grid.h"

#include "tests_common.h"

#include <gtest/gtest.h>
#include <set>
#include <optional>
#include <glm/gtc/constants.hpp>

//using namespace glm;
using namespace std;
using namespace ghassanpl::geometry;
using namespace ghassanpl::geometry::angles;

static_assert(polygon_shape<float, tsegment<float>>);
//static_assert(polygon_shape3d<float, tsegment3d<float>>);

TEST(geometry, basic_types_work)
{
	ray r;
	segment s;
}

TEST(geometry_common, degrees_and_radians_work)
{
	auto d360 = degrees{ 360.0f };
	//EXPECT_EQ(d360.value, ((degrees)radians{ (glm::pi<float>() * 2) }).value);
	static constexpr auto deg = degrees{ (0 + 1) * (360.0f / 4) }.value;
	static_assert(deg == 90.0f);
	auto [qslice_start, qslice_end] = circle_slice<0, 4>;
	static constexpr auto qslice_constexpr = circle_slice<0, 4>;;
	static_assert(qslice_constexpr.first.value == 0.0f);
	static_assert(qslice_constexpr.second.value == 90.0f);
	//char tab[int(qslice2) + 1]{};
	EXPECT_EQ(qslice_start.value, 0.0f);
	EXPECT_EQ(qslice_end.value, 90.0f);
	/*EXPECT_EQ(glm::radians(360.0), (glm::pi<double>() * 2));
	EXPECT_EQ((360.0), degrees(glm::pi<double>() * 2));
	EXPECT_EQ(radians(degrees_t{ 360.0f }).value, (glm::pi<float>() * 2.0f));
	EXPECT_EQ((360.0f), degrees(radians_t{ glm::pi<float>() * 2.0f }).value);
	EXPECT_EQ(0, degrees(radians_t{}).value);
	*/

	EXPECT_EQ(ensure_positive(degrees{ -90.0f }), degrees{ 270.0f });
	EXPECT_EQ(ensure_positive(radians{ -glm::radians(90.0f) }), radians{ glm::radians(270.0f) });
}

TEST(polygon, can_be_created_from_polyshapes)
{
	static_assert(polygon_shape<float, rec2>);
	polygon r = polygon::from_shape(rec2{10, 20, 30, 40});
}

TEST(polygon, free_functions_work_on_ranges_of_vectors)
{
	const std::vector<glm::vec2> poly{ {0,0}, {0,10}, {10,10}, {10, 0} };
	const indexed_triangle t{ {0,1,2} };
	const auto result = calculate_indexed_triangle_area(poly, t);
	EXPECT_EQ(result, 50);
}

using namespace ghassanpl::geometry::squares;

template <typename RESULT_TYPE>
class squares_typed : public ::testing::Test {
public:
	using result_type = RESULT_TYPE;
};

TYPED_TEST_SUITE(squares_typed, integer_types);

TYPED_TEST(squares_typed, metric_distances_work_for_all_types)
{
	using vec = glm::tvec2<TypeParam>;

	EXPECT_EQ(manhattan_distance(vec{ 0,0 }, vec{ 0,1 }), 1);
	EXPECT_EQ(manhattan_distance(vec{ 0,1 }, vec{ 0,0 }), 1);
	EXPECT_EQ(chebyshev_distance(vec{ 0,0 }, vec{ 0,1 }), 1);
	EXPECT_EQ(chebyshev_distance(vec{ 0,1 }, vec{ 0,0 }), 1);
}

TEST(squares, tile_world_grid_functions_work)
{
	using glm::vec2;
	{
		auto snapped = snap_world_pos_to_tile_grid(vec2{ 0,0 }, vec2{ 1,1 });
		EXPECT_EQ(snapped, vec2(0, 0));
	}
	{
		auto snapped = snap_world_pos_to_tile_grid(vec2{ 0.2,0.2 }, vec2{ 1,1 });
		EXPECT_EQ(snapped, vec2(0, 0));
	}
	{
		auto snapped = snap_world_pos_to_tile_grid(vec2{ 0.7,0.7 }, vec2{ 1,1 });
		EXPECT_EQ(snapped, vec2(1, 1));
	}
	{
		auto snapped = snap_world_pos_to_tile_grid(vec2{ -0.2,-0.2 }, vec2{ 1,1 });
		EXPECT_EQ(snapped, vec2(0, 0));
	}
	{
		auto snapped = snap_world_pos_to_tile_grid(vec2{ -0.7,-0.7 }, vec2{ 1,1 });
		EXPECT_EQ(snapped, vec2(-1, -1));
	}
}

TEST(squares, tile_spaces_work)
{
	chebyshev_tile_space space;
	EXPECT_EQ(space.snap_to_grid(glm::vec2{ 10.4, 10.7 }), (glm::vec2{ 10,11 }));
}

TEST(grid, works)
{
	struct tile
	{
		int smth = 5;
	};

	grid<tile> gr;
	EXPECT_EQ(gr.width(), 0);
	EXPECT_EQ(gr.height(), 0);

	gr.reset(10, 20);
	EXPECT_EQ(gr.width(), 10);
	EXPECT_EQ(gr.height(), 20);

	gr.for_each_tile([](glm::ivec2 pos, tile& t) { t.smth = pos.x*pos.y; });
	for (int x=0; x< gr.width(); ++x)
	{
		for (int y = 0; y < gr.height(); ++y)
		{
			EXPECT_EQ(gr.at_index(gr.index(x, y))->smth, x*y) << x << y;
		}
	}

	const auto unblocked = gr.line_cast({ 0,0 }, { 5,5 }, [](glm::ivec2 pos) { return pos != glm::ivec2{1, 1}; }, false);
	EXPECT_FALSE(unblocked);
}

TEST(polar, works)
{
	using namespace glm;
	{
		auto p = euclidean(polar(vec2{10.0f, 20.0f}));
		EXPECT_NEAR(p.x, 10, 0.00001);
		EXPECT_NEAR(p.y, 20, 0.00001);
	}
}

TEST(triangle, basics_work)
{
	{
		ttriangle<float> t{ {0,0}, {1,0}, {0,1} };
		EXPECT_EQ(t.winding(), winding_order::clockwise);
	}
	{
		ttriangle<float> t{ {0,0}, {1,0}, {0,-1} };
		EXPECT_EQ(t.winding(), winding_order::counter_clockwise);
	}
}

TEST(polygon, edges)
{
	polygon p{};
	p.edges();;
}

TEST(segment, intersection_works)
{
	using seg = tsegment<float>;
	constexpr float tol = 1e-5f;

	const auto expect_point = [&](std::optional<glm::vec2> const& r, float x, float y)
	{
		ASSERT_TRUE(r.has_value());
		EXPECT_NEAR(r->x, x, tol);
		EXPECT_NEAR(r->y, y, tol);
	};

	// Crossing diagonals of a square meet at the centre.
	expect_point(seg{ {0,0},{10,10} }.intersection(seg{ {0,10},{10,0} }), 5, 5);

	// Intersection is symmetric.
	expect_point(seg{ {0,10},{10,0} }.intersection(seg{ {0,0},{10,10} }), 5, 5);

	// Lines would cross, but the crossing is outside the segment extents -> no intersection.
	EXPECT_FALSE((seg{ {0,0},{1,0} }.intersection(seg{ {2,-1},{2,1} })).has_value());

	// Parallel but offset -> never meet.
	EXPECT_FALSE((seg{ {0,0},{10,0} }.intersection(seg{ {0,1},{10,1} })).has_value());

	// Shared endpoint / T-junction touches at that point.
	expect_point(seg{ {0,0},{10,0} }.intersection(seg{ {10,0},{10,10} }), 10, 0);

	// Collinear and overlapping -> start of the overlapping sub-segment.
	expect_point(seg{ {0,0},{10,0} }.intersection(seg{ {5,0},{15,0} }), 5, 0);

	// Collinear overlap is independent of the other segment's direction.
	expect_point(seg{ {0,0},{10,0} }.intersection(seg{ {15,0},{5,0} }), 5, 0);

	// Collinear but disjoint -> no intersection.
	EXPECT_FALSE((seg{ {0,0},{10,0} }.intersection(seg{ {20,0},{30,0} })).has_value());

	// Degenerate 'this' (a zero-length point) lying on the other segment.
	expect_point(seg{ {5,0},{5,0} }.intersection(seg{ {0,0},{10,0} }), 5, 0);

	// Symmetric: degenerate 'other'.
	expect_point(seg{ {0,0},{10,0} }.intersection(seg{ {5,0},{5,0} }), 5, 0);

	// Degenerate point off the other segment's line -> no intersection.
	EXPECT_FALSE((seg{ {5,5},{5,5} }.intersection(seg{ {0,0},{10,0} })).has_value());

	// Degenerate point that projects within the span but is off the line -> no intersection.
	EXPECT_FALSE((seg{ {5,0},{5,0} }.intersection(seg{ {0,1},{10,1} })).has_value());

	// Two coincident points intersect at that point.
	expect_point(seg{ {3,4},{3,4} }.intersection(seg{ {3,4},{3,4} }), 3, 4);

	// Two distinct points do not.
	EXPECT_FALSE((seg{ {3,4},{3,4} }.intersection(seg{ {5,6},{5,6} })).has_value());
}

/*

struct tile_data {};
TEST(grid, empty_grid_works)
{
  grid<tile_data> g1{ };
  grid<tile_data> g2{ 0,0, tile_data{} };
  grid<tile_data> g3{ ivec2{0,0}, tile_data{} };
  grid<tile_data> g4{ 0,0};
  grid<tile_data> g5{ ivec2{0,0} };

  map<ivec2, int, decltype([](ivec2 a, ivec2 b) { return std::make_tuple(a.x, a.y) < std::make_tuple(b.x, b.y); })> visited;
  g1.for_each_neighbor({}, [&](ivec2 v) { visited[v]++; });
  EXPECT_TRUE(visited.empty());
}

int main(int argc, char** argv)
{
  //ghassanpl::tests::TestRunner::RunTests();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  return 0;
}
*/