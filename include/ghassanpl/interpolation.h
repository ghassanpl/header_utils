/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <cmath>
#include <type_traits>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtc/constants.hpp>
#include <glm/gtx/spline.hpp>
#undef GLM_ENABLE_EXPERIMENTAL
// TODO: #include "constexpr_math.h"

/// TODO: 
///	- lerp, unlerp, etc
///	- splines
///	- easing functions 
/// - approach

namespace ghassanpl
{
#if GHPL_CPP20
#define GHPL_CONSTRAINT(constraint) constraint
#else
#define GHPL_CONSTRAINT(constraint) typename
#endif
	template <GHPL_CONSTRAINT(std::floating_point) C, GHPL_CONSTRAINT(std::floating_point) D, typename A>
	auto approach_constant(C cur, D dest, A amount)
	{
		using T = decltype(cur);
		return (cur < dest) ? std::min(T(cur + amount), T(dest)) : std::max(T(cur - amount), T(dest));
	}

	template <GHPL_CONSTRAINT(std::floating_point) C, GHPL_CONSTRAINT(std::floating_point) D, typename S>
	auto approach_soft(C cur, D dest, S speed)
	{
		return approach_constant(cur, dest, std::abs(dest - cur) * speed);
	}

	template <size_t N, typename T, typename S>
	auto approach_soft(glm::vec<N, T> const& cur, glm::vec<N, T> dest, S speed) -> glm::vec<N, T>
	{
		for (int i=0;i<int(N); ++i)
			dest[i] = approach_soft(cur[i], dest[i], speed);
		return dest;
	}
	
	template <GHPL_CONSTRAINT(std::floating_point) C, GHPL_CONSTRAINT(std::floating_point) D, typename S, typename P = int>
	auto approach_fast(C cur, D dest, S speed, P power = 2)
	{
		return approach_constant(cur, dest, std::pow(dest - cur, power) * speed);
	}

	template <size_t N, typename T, typename S, typename P = int>
	auto approach_fast(glm::vec<N, T> const& cur, glm::vec<N, T> dest, S speed, P power = 2) -> glm::vec<N, T>
	{
		for (int i = 0; i<int(N); ++i)
			dest[i] = approach_fast(cur[i], dest[i], speed, power);
		return dest;
	}

	namespace splines
	{
		template <typename P, typename T>
		inline auto bezier(P const& a, P const& b, P const& c, T t) { return glm::mix(glm::mix(a, b, t), glm::mix(b, c, t), t); }

		template <typename P, typename T>
		inline auto bezier(P const& a, P const& b, P const& c, P const& d, T t) { return bezier(glm::mix(a, b, t), glm::mix(b, c, t), glm::mix(c, d, t), t); }

		using glm::catmullRom;
		using glm::cubic;
		using glm::hermite;
	}

	namespace waves
	{
		/// TODO:
		/// - blink/square wave
		/// - sawtooth

		template <typename T = double>
		struct wave_properties
		{
			static_assert(std::is_floating_point_v<T>);

			T phase_in_cycles = 0;
			T min_val = 0;
			T max_val = 1;
			T period = 1;

			constexpr wave_properties shifted_by_cycles(T ph) const { wave_properties result = *this; result.phase_in_cycles += ph; return result; }
			constexpr wave_properties shifted_by_time(T ph) const { wave_properties result = *this; result.phase_in_cycles += ph / period; return result; }
			constexpr wave_properties with_cycle_phase(T ph) const { wave_properties result = *this; result.phase_in_cycles = ph; return result; }
			constexpr wave_properties with_time_phase(T ph) const { wave_properties result = *this; result.phase_in_cycles = ph / period; return result; }
			constexpr wave_properties with_frequency(T freq) const { wave_properties result = *this; result.period = 1/freq; return result; }
			constexpr wave_properties with_period(T period) const { wave_properties result = *this; result.period = period; return result; }
			constexpr wave_properties between(T min, T max) const { wave_properties result = *this; result.min_val = min; result.max_val = max; return result; }

			template <T min = 0.0, T max = 1.0>
			constexpr T rescale_result(T v) const
			{
				return ((v - min) / (max - min)) * (max_val - min_val) + min_val;
			}

			constexpr T alpha_from_time(T v) const
			{
				return (v / period) + phase_in_cycles;
			}
		};

		template <typename T> constexpr wave_properties<T> with_cycle_phase(T ph) { wave_properties<T> result = {}; result.phase_in_cycles = ph; return result; }
		template <typename T> constexpr wave_properties<T> with_frequency(T freq) { wave_properties<T> result = {}; result.period = 1 / freq; return result; }
		template <typename T> constexpr wave_properties<T> with_period(T period) { wave_properties<T> result = {}; result.period = period; return result; }
		template <typename T> constexpr wave_properties<T> between(T min, T max) { wave_properties<T> result = {}; result.min_val = min; result.max_val = max; return result; }

		template <typename T = double>
		inline T triangle(T t, wave_properties<T> properties = {})
		{
			auto u = std::fmod(properties.alpha_from_time(t), T(1.0));
			if (u < T(0)) u += T(1);
			return properties.rescale_result(T(1.0) - std::abs(u - T(0.5)) * T(2));
		}

		template <typename T>
		inline T sin(T t, wave_properties<T> properties = {})
		{
			return properties.template rescale_result<T(-1.0), T(1.0)>(std::sin(properties.alpha_from_time(t) * glm::two_pi<T>()));
		}
	}
}
