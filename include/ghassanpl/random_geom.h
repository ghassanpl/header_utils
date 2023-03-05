/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "random.h"
#include "rec2.h"

namespace ghassanpl::random
{
	template <std::floating_point T = float, typename RANDOM = std::default_random_engine>
	T radians(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<T> dist{ T{}, glm::two_pi<T>() };
		return dist(rng);
	}

	template <std::floating_point T = float, typename RANDOM = std::default_random_engine>
	T degrees(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<T> dist{ T{}, T{360} };
		return dist(rng);
	}

	template <std::floating_point T = float, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> unit_vector(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return glm::rotate(glm::tvec2<T>{ T{ 1 }, T{ 0 } }, radians<T>(rng));
	}

	template <typename T, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> in(trec2<T> const& rect, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return { range(rect.p1.x, rect.p2.x, rng), range(rect.p1.y, rect.p2.y, rng) };
	}

	/// TODO: in(circle), in(poly)?, on(rect), on(circle)

	template <typename RANDOM = std::default_random_engine>
	glm::ivec2 neighbor(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM = std::default_random_engine>
	glm::ivec2 diagonal_neighbor(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 1.0f };
		case 1: return { -1.0f, 1.0f };
		case 2: return { -1.0f, -1.0f };
		case 3: return { 1.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM = std::default_random_engine>
	glm::ivec2 surrounding(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 7 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		case 4: return { 1.0f, 1.0f };
		case 5: return { -1.0f, 1.0f };
		case 6: return { -1.0f, -1.0f };
		case 7: return { 1.0f, -1.0f };
		}
		return {};
	}
}