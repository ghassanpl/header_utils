/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/interpolation.h"

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace ghassanpl::waves;

TEST(wave_properties, builders_work)
{
	const wave_properties<double> def{};
	EXPECT_EQ(def.phase_in_cycles, 0.0);
	EXPECT_EQ(def.min_val, 0.0);
	EXPECT_EQ(def.max_val, 1.0);
	EXPECT_EQ(def.period, 1.0);

	EXPECT_EQ(def.with_frequency(4.0).period, 0.25);

	const auto ranged = def.between(2.0, 6.0);
	EXPECT_EQ(ranged.min_val, 2.0);
	EXPECT_EQ(ranged.max_val, 6.0);

	/// shifted_by_* are relative: repeated shifts accumulate.
	EXPECT_EQ(def.shifted_by_cycles(0.25).phase_in_cycles, 0.25);
	EXPECT_EQ(def.shifted_by_cycles(0.25).shifted_by_cycles(0.25).phase_in_cycles, 0.5);

	/// shifted_by_time converts through the current period.
	const wave_properties<double> two_sec{ .period = 2.0 };
	EXPECT_EQ(two_sec.shifted_by_time(1.0).phase_in_cycles, 0.5);
	EXPECT_EQ(two_sec.shifted_by_time(1.0).shifted_by_time(1.0).phase_in_cycles, 1.0);
}

TEST(wave_properties, alpha_and_rescale_work)
{
	const wave_properties<double> props{ .phase_in_cycles = 0.25, .period = 2.0 };
	EXPECT_DOUBLE_EQ(props.alpha_from_time(1.0), 0.75); /// 1/2 + 0.25

	const auto ranged = wave_properties<double>{}.between(2.0, 6.0);
	/// Default input range [0, 1]:
	EXPECT_DOUBLE_EQ(ranged.rescale_result(0.0), 2.0);
	EXPECT_DOUBLE_EQ(ranged.rescale_result(0.5), 4.0);
	EXPECT_DOUBLE_EQ(ranged.rescale_result(1.0), 6.0);
	/// Explicit input range [-1, 1] (as used by sin):
	EXPECT_DOUBLE_EQ((ranged.rescale_result<-1.0, 1.0>(-1.0)), 2.0);
	EXPECT_DOUBLE_EQ((ranged.rescale_result<-1.0, 1.0>(0.0)), 4.0);
	EXPECT_DOUBLE_EQ((ranged.rescale_result<-1.0, 1.0>(1.0)), 6.0);
}

TEST(waves, triangle_works)
{
	/// Default wave: rises 0 -> 1 over the first half period, falls back over the second.
	EXPECT_DOUBLE_EQ(triangle(0.0), 0.0);
	EXPECT_DOUBLE_EQ(triangle(0.25), 0.5);
	EXPECT_DOUBLE_EQ(triangle(0.5), 1.0);
	EXPECT_DOUBLE_EQ(triangle(0.75), 0.5);
	EXPECT_DOUBLE_EQ(triangle(1.0), 0.0);

	/// Periodicity: equivalent points one (or more) periods apart match.
	EXPECT_NEAR(triangle(0.3), triangle(1.3), 1e-9);
	EXPECT_NEAR(triangle(0.3), triangle(5.3), 1e-9);

	/// Negative time wraps instead of escaping the output range.
	EXPECT_DOUBLE_EQ(triangle(-0.25), 0.5);
	EXPECT_DOUBLE_EQ(triangle(-0.5), 1.0);
	EXPECT_NEAR(triangle(-1.7), triangle(0.3), 1e-9);

	/// The result never leaves [min_val, max_val], even for negative times.
	for (int i = -30; i <= 30; ++i)
	{
		const auto v = triangle(i * 0.1);
		EXPECT_GE(v, 0.0) << "at t = " << i * 0.1;
		EXPECT_LE(v, 1.0) << "at t = " << i * 0.1;
	}

	/// Output range mapping.
	const auto ranged = wave_properties<double>{}.between(2.0, 6.0);
	EXPECT_DOUBLE_EQ(triangle(0.0, ranged), 2.0);
	EXPECT_DOUBLE_EQ(triangle(0.25, ranged), 4.0);
	EXPECT_DOUBLE_EQ(triangle(0.5, ranged), 6.0);

	/// A 2-second period peaks at t = 1 and returns to min at t = 2.
	const wave_properties<double> two_sec{ .period = 2.0 };
	EXPECT_DOUBLE_EQ(triangle(1.0, two_sec), 1.0);
	EXPECT_DOUBLE_EQ(triangle(2.0, two_sec), 0.0);

	/// Positive phase shifts the wave earlier: half a cycle puts the peak at t = 0.
	EXPECT_DOUBLE_EQ(triangle(0.0, wave_properties<double>{}.shifted_by_cycles(0.5)), 1.0);
	EXPECT_DOUBLE_EQ(triangle(0.0, two_sec.shifted_by_time(1.0)), 1.0);
}

TEST(waves, sin_works)
{
	/// Default wave: starts at the midpoint, peaks a quarter period in.
	EXPECT_NEAR(waves::sin(0.0), 0.5, 1e-9);
	EXPECT_NEAR(waves::sin(0.25), 1.0, 1e-9);
	EXPECT_NEAR(waves::sin(0.5), 0.5, 1e-9);
	EXPECT_NEAR(waves::sin(0.75), 0.0, 1e-9);
	EXPECT_NEAR(waves::sin(1.0), 0.5, 1e-9);

	/// The time between successive peaks is exactly one period.
	/// (Regression: alpha must be scaled by 2*pi, not 2.)
	EXPECT_NEAR(waves::sin(1.25), 1.0, 1e-9);
	EXPECT_NEAR(waves::sin(0.3), waves::sin(1.3), 1e-9);

	/// Negative time works (sin needs no wrapping).
	EXPECT_NEAR(waves::sin(-0.75), 1.0, 1e-9);

	/// Output range mapping, including negative bounds.
	const auto ranged = wave_properties<double>{}.between(-3.0, 3.0);
	EXPECT_NEAR(waves::sin(0.25, ranged), 3.0, 1e-9);
	EXPECT_NEAR(waves::sin(0.75, ranged), -3.0, 1e-9);
	EXPECT_NEAR(waves::sin(0.0, ranged), 0.0, 1e-9);

	/// A 2-second period peaks at t = 0.5.
	const wave_properties<double> two_sec{ .period = 2.0 };
	EXPECT_NEAR(waves::sin(0.5, two_sec), 1.0, 1e-9);

	/// A quarter-cycle shift puts the peak at t = 0.
	EXPECT_NEAR(waves::sin(0.0, wave_properties<double>{}.shifted_by_cycles(0.25)), 1.0, 1e-9);
}

TEST(waves, float_instantiation_works)
{
	/// Exercises the float path: float NTTPs in rescale_result and the
	/// dependent .template call in sin.
	EXPECT_NEAR(triangle(0.5f), 1.0f, 1e-5f);
	EXPECT_NEAR(triangle(-0.25f), 0.5f, 1e-5f);
	EXPECT_NEAR(waves::sin(0.25f), 1.0f, 1e-5f);
	EXPECT_NEAR(waves::sin(0.75f, wave_properties<float>{}.between(2.0f, 6.0f)), 2.0f, 1e-4f);
}
