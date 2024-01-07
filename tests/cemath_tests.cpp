/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/constexpr_math.h"
#include "tests_common.h"

#include <gtest/gtest.h>

using namespace ghassanpl;

volatile float target = 0.0f; /// volatile to surpress any constexpr evaluations

TEST(cemath_functions, work)
{
	{
		constexpr float ce = cem::abs(-50.0f);
		static_assert(ce == 50.0f, "cem::abs failed");

		target = -50.0f;
		const float nc = cem::abs(target);
		EXPECT_EQ(nc, ce);
	}
	{
		constexpr float ce = cem::ceil(53.2f);
		constexpr float ce2 = cem::ceil(-53.2f);
		static_assert(ce == 54.0f, "cem::ceil failed");
		static_assert(ce2 == -53.0f, "cem::ceil failed");

		target = 53.2f;
		const float nc = cem::ceil(target);
		EXPECT_EQ(nc, ce);

		target = -53.2f;
		const float nc2 = cem::ceil(target);
		EXPECT_EQ(nc2, ce2);
	}
	{
		constexpr float ce = cem::floor(53.2f);
		constexpr float ce2 = cem::floor(-53.2f);
		static_assert(ce == 53.0f, "cem::floor failed");
		static_assert(ce2 == -54.0f, "cem::floor failed");

		target = 53.2f;
		const float nc = cem::floor(target);
		EXPECT_EQ(nc, ce);

		target = -53.2f;
		const float nc2 = cem::floor(target);
		EXPECT_EQ(nc2, ce2);
	}
	{
		constexpr float ce = cem::fmod(3.5f, 2.0f);
		constexpr float ce2 = cem::fmod(-3.5f, 2.0f);
		static_assert(ce == 1.5f, "cem::fmod failed");
		static_assert(ce2 == -1.5f, "cem::fmod failed");

		target = 3.5f;
		const float nc = cem::fmod(target, 2.0f);
		EXPECT_EQ(nc, ce);

		target = -3.5f;
		const float nc2 = cem::fmod(target, 2.0f);
		EXPECT_EQ(nc2, ce2);
	}
	{
		constexpr float ce = cem::fma(3.5f, 2.0f, -10.0f);
		static_assert(ce == -3.0f, "cem::fma failed");

		target = 3.5f;
		const float nc = cem::fma(target, 2.0f, -10.0f);
		EXPECT_EQ(nc, ce);
	}
	{
		constexpr float ce = cem::pow(3.5f, 2u);
		static_assert(ce == 12.25f, "cem::pow failed");

		target = 3.5f;
		const float nc = cem::pow(target, 2u);
		EXPECT_EQ(nc, ce);
	}

	{
		constexpr float ce = cem::pow<2>(3.5f);
		static_assert(ce == 12.25f, "cem::pow failed");

		target = 3.5f;
		const float nc = cem::pow<2>(target);
		EXPECT_EQ(nc, ce);
	}
	{
		{ constexpr auto val = cem::signbit(-3.5f); static_assert(true == val); }
		{ constexpr auto val = cem::signbit(3.5f); static_assert(false == val); }
		{ constexpr auto val = cem::signbit(std::numeric_limits<float>::quiet_NaN()); static_assert(false == val); }
		{ constexpr auto val = cem::signbit(-std::numeric_limits<float>::quiet_NaN()); static_assert(true == val); }
		{ constexpr auto val = cem::signbit(INFINITY); static_assert(false == val); }
		{ constexpr auto val = cem::signbit(-INFINITY); static_assert(true == val); }
		{ constexpr auto val = cem::signbit(-0.0f); static_assert(true == val); }
		{ constexpr auto val = cem::signbit(0.0f); static_assert(false == val); }

		target = -3.5f;     EXPECT_TRUE(cem::signbit(target));
		target = 3.5f;      EXPECT_FALSE(cem::signbit(target));
		target = NAN;       EXPECT_FALSE(cem::signbit(target));
		target = -NAN;      EXPECT_TRUE(cem::signbit(target));
		target = INFINITY;  EXPECT_FALSE(cem::signbit(target));
		target = -INFINITY; EXPECT_TRUE(cem::signbit(target));
		target = -0.0f;     EXPECT_TRUE(cem::signbit(target));
		target = 0.0f;      EXPECT_FALSE(cem::signbit(target));
	}
}


TEST(cemath_functions, work_on_heterogenous_parameters)
{
	{ constexpr auto val = cem::pow(20.0f, 2u); static_assert(std::same_as<const float, decltype(val)> && val == 400.0f); }
	{ constexpr auto val = cem::pow(20.0, 2u); static_assert(std::same_as<const double, decltype(val)> && val == 400.0); }
	{ constexpr auto val = cem::pow(20, 2u); static_assert(std::same_as<const int, decltype(val)> && val == 400); }

	{ constexpr auto val = cem::fma(10.0f, 2, -3.); static_assert(std::same_as<const double, decltype(val)> && val == 17.0); }
	{ constexpr auto val = cem::fma(10.0f, 2.0f, -3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 17.0); }

	{ constexpr auto val = cem::fmod(10.0f, 3); static_assert(std::same_as<const float, decltype(val)> && val == 1.0f); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0); static_assert(std::same_as<const double, decltype(val)> && val == 1.0); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 1.0); }


	{ auto val = cem::pow(20.0f, 2u); static_assert(std::same_as<float, decltype(val)>); EXPECT_EQ(val, 400.0f); }
	{ auto val = cem::pow(20.0, 2u); static_assert(std::same_as<double, decltype(val)>); EXPECT_EQ(val, 400.0); }
	{ auto val = cem::pow(20, 2u); static_assert(std::same_as<int, decltype(val)>); EXPECT_EQ(val, 400); }

	{ constexpr auto val = cem::fma(10.0f, 2, -3.); static_assert(std::same_as<const double, decltype(val)> && val == 17.0); }
	{ constexpr auto val = cem::fma(10.0f, 2.0f, -3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 17.0); }

	{ constexpr auto val = cem::fmod(10.0f, 3); static_assert(std::same_as<const float, decltype(val)> && val == 1.0f); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0); static_assert(std::same_as<const double, decltype(val)> && val == 1.0); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 1.0); }
}