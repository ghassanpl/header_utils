/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

namespace ghassanpl
{
#if __cplusplus >= 202002L
	template <typename T>
	concept mutex_type = requires(T t)
	{
		{ t.lock() };
		{ t.try_lock() } -> std::convertible_to<bool>;
		{ t.unlock() };
	};
#else
#define mutex_type typename
#endif

	template <mutex_type MUTEX_TYPE, typename FUNC, typename... ARGS>
	auto under_protection(MUTEX_TYPE& m, FUNC&& func, ARGS&&... args)
	{
		std::lock_guard guard{ m };
		if constexpr (std::invocable<FUNC, MUTEX_TYPE&, ARGS...>)
			return func(m, std::forward<ARGS>(args)...);
		else
			return func(std::forward<ARGS>(args)...);
	}

	template <mutex_type MUTEX_TYPE, typename T>
	[[nodiscard]] auto protected_copy(MUTEX_TYPE& m, T&& t)
	{
		std::lock_guard guard{ m };
		return std::forward<T>(t);
	}

}