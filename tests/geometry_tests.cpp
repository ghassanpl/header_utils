//#include "../include/ghassanpl/geometry/angles.h"
#include "../include/ghassanpl/geometry/square_grid.h"
#include "../include/ghassanpl/geometry/ellipse.h"
#include "../include/ghassanpl/geometry/polygon.h"
#include "../include/ghassanpl/geometry/segment.h"

#include "../include/ghassanpl/geometry/points.h"
#include "../include/ghassanpl/geometry/rectangles.h"
#include "../include/ghassanpl/functional.h"
//#include "../include/ghassanpl/geometry/squares.h"
//#include "../include/ghassanpl/geometry/grid_algorithms.h"
//#include "../include/ghassanpl/geometry/block_navigation_grid.h"

#include <gtest/gtest.h>
#include <set>
#include <glm/gtc/constants.hpp>

//using namespace glm;
using namespace std;
using namespace ghassanpl::geometry;
using namespace ghassanpl::geometry::angles;

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
}

using namespace ghassanpl::geometry::squares;
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

TEST(polar, works)
{
	using namespace glm;
	{
		auto p = euclidean(polar(vec2{10, 20}));
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