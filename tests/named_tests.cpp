/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/named.h"
#include "../include/ghassanpl/geometry/geometry_common.h"

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace ghassanpl::geometry;
using glm::ivec2;

template <typename T, typename U = T, typename RESULT = T> concept addable = requires { { T{} + U{} } -> std::same_as<RESULT>; };
template <typename T, typename U = T, typename RESULT = T> concept subtractable = requires { { T{} - U{} } -> std::same_as<RESULT>; };
template <typename T, typename U = T, typename RESULT = T> concept multipliable = requires { { T{} * U{} }-> std::same_as<RESULT>; };

TEST(named, named_location_and_displacement_traits_work)
{
	using vector = named<ivec2, "vector", traits::displacement>;
	using point = named<ivec2, "point", traits::location, traits::is_location_of<vector>>;

	static_assert(traits::applies_to<point, traits::location>);
	static_assert(!traits::applies_to<point, traits::displacement>);
	static_assert(traits::applies_to<vector, traits::displacement>);
	static_assert(!traits::applies_to<point, traits::displacement>);
	static_assert(is_named_v<point>);
	static_assert(is_named_v<vector>);
	static_assert(!(addable<point>));
	static_assert(!(addable<point, ivec2>));
	static_assert((addable<point, vector>));
	static_assert((addable<vector, point, point>));
	static_assert((addable<vector, vector, vector>));
	static_assert(!(addable<vector, ivec2>)); /// You need to always specify units

	static_assert(subtractable<vector>);
	static_assert(!subtractable<point, point, point>);
	static_assert(subtractable<point, point, vector>);

	constexpr auto trait_ti = point::find_displacement_type_impl(traits::is_location_of<vector>{});
	using type = std::remove_cvref_t<typename decltype(trait_ti)::type>;

	EXPECT_EQ((point{ 5,5 } - point{ 2,2 }), (vector{ 3,3 }));
	EXPECT_EQ((point{ 5,5 } + vector{ 2,2 }), (point{ 7,7 }));
	
	static_assert(!(multipliable<point>));
	static_assert(!(multipliable<vector>));

	static_assert(!(multipliable<point, ivec2>)); /// You can't multiply locations or dates or points
	static_assert((multipliable<vector, ivec2>)); /// But you can multiply displacements/magnitudes, like durations, etc.

	static_assert(heading::has_trait<traits::location>);
	static_assert(degrees::template has_trait<traits::displacement>);
	static_assert(traits::named_is_displacement_of<degrees, heading>);
	static_assert(traits::addable::applies_to<heading, degrees>);
	static_assert((addable<degrees, heading, heading>));
	static_assert((addable<heading, degrees, heading>));
	static_assert((addable<degrees, degrees, degrees>));
	degrees displacement;
	heading location;
	location + displacement;
	displacement + location;
	//h + d;
}