/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/constexpr_math.h"
#include "tests_common.h"

#include <gtest/gtest.h>
#include <print>
#include "test_system.h"

using namespace ghassanpl;

volatile float target = 0.0f; /// volatile to surpress any constexpr evaluations

constexpr std::array<double, 505> GenerateFloats()
{
	std::array<double, 505> floats;
	for (int i = 250; i > -250; --i)
	{
		if (i != 0)
			floats[250-i] = (100.0 / i) * 500.0;
		else
			floats[250-i] = 0;
	}
	floats[500] = 0.0;
	floats[501] = -0.0;
	floats[502] = std::numeric_limits<double>::infinity();
	floats[503] = -std::numeric_limits<double>::infinity();
	floats[504] = std::numeric_limits<double>::quiet_NaN();
	return floats;
}

constexpr auto Floats = GenerateFloats();

#define DO_FUNC(func) \
	[]() { std::array<decltype(cem::func(0.0)), 505> result; for (size_t i=0; i<505; ++i) { result[i] = cem::func(Floats[i]); } return result; }()

#define DO_TEST(func) \
	ShouldForValuesInRange(0, 505, ReturnTheSameValueFor_##func) { \
		DoesReturnTheSameValueFor_##func.SetContextValue("ForValue", std::to_string(Floats[Value])); \
		DoesReturnTheSameValueFor_##func.WhenEqual(cem_##func##_results[Value], std::func(Floats[Value])); \
	}

UnderTest(ghassanpl::constexpr_math)
{
	CheckingIfIt("'s functions return the same value in constexpr as in non-constexpr")
	{
		constexpr auto cem_abs_results = DO_FUNC(abs);
		constexpr auto cem_floor_results = DO_FUNC(floor);
		constexpr auto cem_ceil_results = DO_FUNC(ceil);
		constexpr auto cem_trunc_results = DO_FUNC(trunc);
		constexpr auto cem_signbit_results = DO_FUNC(signbit);
		constexpr auto cem_sqrt_results = DO_FUNC(sqrt);

		DO_TEST(abs);
		DO_TEST(floor);
		DO_TEST(ceil);
		DO_TEST(trunc);
		DO_TEST(signbit);
		DO_TEST(sqrt);

		/// fma is tested by fmod

		constexpr auto cem_fmod_results = []() { 
			std::array<decltype(cem::fmod(0.0, 0.0)), 505> result; 
			for (size_t i = 0; i<505; ++i) {
				const auto mul = (Floats[i] != Floats[i]) ? Floats[i] : Floats[i] * Floats[i];
				result[i] = cem::fmod(Floats[i], mul);
			} 
			return result; 
		}();
		ShouldForValuesInRange(0, 505, ReturnTheSameValueFor_fmod) {
			DoesReturnTheSameValueFor_fmod.SetContextValue("ForValueA", std::to_string(Floats[Value]));
			DoesReturnTheSameValueFor_fmod.SetContextValue("ForValueB", std::to_string(Floats[Value] * Floats[Value]));
			DoesReturnTheSameValueFor_fmod.WhenEqual(cem_fmod_results[Value], std::fmod(Floats[Value], Floats[Value] * Floats[Value]));
		};

		constexpr auto cem_pow_results = []() {
			std::array<decltype(cem::pow(0.0, 2.2)), 505> result;
			for (size_t i = 0; i < 505; ++i) {
				const auto mul = (Floats[i] != Floats[i]) ? Floats[i] : Floats[i] * Floats[i];
				result[i] = cem::pow(Floats[i], mul);
			}
			return result;
		}();
		ShouldForValuesInRange(0, 505, ReturnTheSameValueFor_pow) {
				DoesReturnTheSameValueFor_pow.SetContextValue("ForValueA", std::to_string(Floats[Value]));
				DoesReturnTheSameValueFor_pow.SetContextValue("ForValueB", std::to_string(Floats[Value] * Floats[Value]));
				DoesReturnTheSameValueFor_pow.WhenEqual(cem_pow_results[Value], std::pow(Floats[Value], Floats[Value] * Floats[Value]));
		};
	}
}

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
		constexpr float ce = cem::pow(3.5f, 2.0f);
		static_assert(ce == 12.25f, "cem::pow failed");

		target = 3.5f;
		const float nc = cem::pow(target, 2u);
		EXPECT_EQ(nc, ce);
	}
	
	/*
	{
		constexpr float ce = cem::pow<2>(3.5f);
		static_assert(ce == 12.25f, "cem::pow failed");

		target = 3.5f;
		const float nc = cem::pow<2>(target);
		EXPECT_EQ(nc, ce);
	}
	*/
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
	{ constexpr auto val = cem::pow(20.0f, 2.0f); static_assert(std::same_as<const float, decltype(val)> && val == 400.0f); }
	{ constexpr auto val = cem::pow(20.0f, 2.0); static_assert(std::same_as<const double, decltype(val)> && val == 400.0f); }
	{ constexpr auto val = cem::pow(20.0, 2.0); static_assert(std::same_as<const double, decltype(val)> && val == 400.0); }
	{ constexpr auto val = cem::pow(20, 2); static_assert(std::same_as<const int, decltype(val)> && val == 400); }

	{ constexpr auto val = cem::sqrt(400.0f); static_assert(std::same_as<const float, decltype(val)> && val == 20.0f); }
	{ constexpr auto val = cem::sqrt(400.0); static_assert(std::same_as<const double, decltype(val)> && val == 20.0); }
	{ constexpr auto val = cem::sqrt(400); static_assert(std::same_as<const double, decltype(val)> && val == 20.0f); }

	{ constexpr auto val = cem::fma(10.0f, 2, -3.); static_assert(std::same_as<const double, decltype(val)> && val == 17.0); }
	{ constexpr auto val = cem::fma(10.0f, 2.0f, -3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 17.0); }

	{ constexpr auto val = cem::fmod(10.0f, 3); static_assert(std::same_as<const float, decltype(val)> && val == 1.0f); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0); static_assert(std::same_as<const double, decltype(val)> && val == 1.0); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 1.0); }


	{ auto val = cem::pow(20.0f, 2u); static_assert(std::same_as<float, decltype(val)>); EXPECT_EQ(val, 400.0f); }
	{ auto val = cem::pow(20.0, 2u); static_assert(std::same_as<double, decltype(val)>); EXPECT_EQ(val, 400.0); }
	{ auto val = cem::pow(20, 2u); static_assert(std::same_as<unsigned int, decltype(val)>); EXPECT_EQ(val, 400); }

	{ constexpr auto val = cem::fma(10.0f, 2, -3.); static_assert(std::same_as<const double, decltype(val)> && val == 17.0); }
	{ constexpr auto val = cem::fma(10.0f, 2.0f, -3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 17.0); }

	{ constexpr auto val = cem::fmod(10.0f, 3); static_assert(std::same_as<const float, decltype(val)> && val == 1.0f); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0); static_assert(std::same_as<const double, decltype(val)> && val == 1.0); }
	{ constexpr auto val = cem::fmod(10.0f, 3.0f); static_assert(std::same_as<const float, decltype(val)> && val == 1.0); }
}
