/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <mutex>
#include <shared_mutex>

namespace ghassanpl
{
#if __cplusplus >= 202002L
	template <typename T>
	concept mutexlike = requires(T t)
	{
		{ t.lock() };
		{ t.try_lock() } -> std::convertible_to<bool>;
		{ t.unlock() };
	};
#else
#define mutexlike typename
#endif

	template <mutexlike MUTEX_TYPE, typename FUNC, typename... ARGS>
	auto under_protection(MUTEX_TYPE& m, FUNC&& func, ARGS&&... args)
	{
		std::lock_guard<MUTEX_TYPE> guard{ m };
		if constexpr (std::is_invocable_v<FUNC, MUTEX_TYPE&, ARGS...>)
			return func(m, std::forward<ARGS>(args)...);
		else
			return func(std::forward<ARGS>(args)...);
	}

	template <mutexlike MUTEX_TYPE, typename T>
	[[nodiscard]] auto protected_copy(MUTEX_TYPE& m, T&& t)
	{
		std::lock_guard guard{ m };
		return std::forward<T>(t);
	}


	template <typename T, mutexlike MUTEX_TYPE = std::mutex>
	struct protected_object
	{
		using mutex_type = MUTEX_TYPE;

		T get() const
		{
			return protected_copy(m_mutex, m_value);
		}

		template <typename U = T>
		void set(U&& value)
		{
			under_protection(m_mutex, [&]() { m_value = std::forward<U>(value); });
		}

		/// `func` will be executed under the protection of the mutex, and will receive a reference to the value.
		template <typename FUNC>
		void mutate_in_place(FUNC&& func)
		{
			under_protection(m_mutex, func, m_value);
		}

		/// `func` will receive a reference to a copy of the value,
		/// and will NOT be executed under the protection of the mutex.
		/// After `func` returns, the mutated copy will be set as the new value.
		template <typename FUNC>
		void mutate_by_copy(FUNC&& func)
		{
			auto copy = this->get();
			func(copy);
			this->set(std::move(copy));
		}

	protected:

		T m_value;
		mutable mutex_type m_mutex;
	};

	template <typename T>
	struct write_protected_object
	{
		using mutex_type = std::shared_mutex;

		T get() const
		{
			std::shared_lock lock{ m_mutex };
			return m_value;
		}

		template <typename U = T>
		void set(U&& value)
		{
			under_protection(m_mutex, [&]() { m_value = std::forward<U>(value); });
		}

		/// `func` will be executed under the protection of the mutex, and will receive a reference to the value.
		template <typename FUNC>
		void mutate_in_place(FUNC&& func)
		{
			under_protection(m_mutex, func, m_value);
		}

		/// `func` will receive a reference to a copy of the value,
		/// and will NOT be executed under the protection of the mutex.
		/// After `func` returns, the mutated copy will be set as the new value.
		template <typename FUNC>
		void mutate_by_copy(FUNC&& func)
		{
			auto copy = this->get();
			func(copy);
			this->set(std::move(copy));
		}

	protected:

		T m_value;
		mutable mutex_type m_mutex;
	};

#ifdef mutexlike
#undef mutexlike
#endif
}