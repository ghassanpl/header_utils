/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp20.h"
#include <mutex>
#include <shared_mutex>
#include <chrono>

namespace ghassanpl
{
	/// \defgroup Threading Threading
	/// Helpers for multi-threaded code.
	/// @{

	/// Anything that's lockable
	template <typename T>
	concept mutexlike = requires(T t)
	{
		{ t.lock() };
		{ t.try_lock() } -> std::convertible_to<bool>;
		{ t.unlock() };
	};

	/// Any mutexlike that can also be attempted to be locked for some time
	template <typename T>
	concept timed_mutexlike = mutexlike<T> && requires(T t)
	{
		{ t.try_lock_for(std::chrono::seconds(1)) } -> std::convertible_to<bool>;
		{ t.try_lock_until(std::chrono::high_resolution_clock::now()) } -> std::convertible_to<bool>;
	};

	/// RW mutex (read access can be shared, write access is exclusive)
	template <typename T>
	concept shared_mutexlike = mutexlike<T> && requires(T t)
	{
		{ t.lock_shared() };
		{ t.try_lock_shared() } -> std::convertible_to<bool>;
		{ t.unlock_shared() };
	};

	/// Runs `func` under the protection of mutexlike `m`
	template <mutexlike MUTEX_TYPE, typename FUNC, typename... ARGS>
	auto under_protection(MUTEX_TYPE& m, FUNC&& func, ARGS&&... args)
	{
		std::unique_lock<MUTEX_TYPE> guard{ m };
		if constexpr (std::is_invocable_v<FUNC, MUTEX_TYPE&, ARGS...>)
			return func(m, std::forward<ARGS>(args)...);
		else
			return func(std::forward<ARGS>(args)...);
	}

	/// Runs `func` under the read (shared) protection of mutexlike `m`
	template <shared_mutexlike MUTEX_TYPE, typename FUNC, typename... ARGS>
	auto under_read_protection(MUTEX_TYPE& m, FUNC&& func, ARGS&&... args)
	{
		std::shared_lock<MUTEX_TYPE> guard{ m };
		if constexpr (std::is_invocable_v<FUNC, MUTEX_TYPE&, ARGS...>)
			return func(m, std::forward<ARGS>(args)...);
		else
			return func(std::forward<ARGS>(args)...);
	}

	/// Creates a copy of `t` under the protection of mutexlike `m`
	template <mutexlike MUTEX_TYPE, typename T>
	[[nodiscard]] auto protected_copy(MUTEX_TYPE& m, T&& t)
	{
		using lock_type = std::conditional_t<shared_mutexlike<MUTEX_TYPE>, std::shared_lock<MUTEX_TYPE>, std::unique_lock<MUTEX_TYPE>>;
		lock_type guard{ m };
		return t;
	}

	/// Moves `t` under the protection of mutexlike `m`
	template <mutexlike MUTEX_TYPE, typename T>
	[[nodiscard]] auto protected_move(MUTEX_TYPE& m, T&& t)
	{
		std::unique_lock guard{ m };
		return std::move(t);
	}

	/// Exchanges `t` with `u` and returns the value of `t`, all under the protection of mutexlike `m`
	template <mutexlike MUTEX_TYPE, typename T, typename U>
	[[nodiscard]] auto protected_exchange(MUTEX_TYPE& m, T& t, U&& u) -> T
	{
		std::unique_lock guard{ m };
		return std::exchange<T>(t, std::forward<U>(u));
	}

	/// A class that stores an object to which access is protected via a mutexlike; all operations are thread-safe unless specified otherwise.
	template <typename T, mutexlike MUTEX_TYPE = std::mutex>
	struct protected_object
	{
		// TODO: Timed versions of this class's functions
		// TODO: std::optional<T> try_get_for(...);

		using object_type = T;
		using reference = object_type&;
		using const_reference = object_type const&;
		using mutex_type = MUTEX_TYPE;
		static constexpr bool is_shared = shared_mutexlike<mutex_type>;
		static constexpr bool is_timed = timed_mutexlike<mutex_type>;

		/// Returns a copy of the object
		T get() const
		{
			return protected_copy(m_mutex, m_value);
		}

		/// Takes the value of the object via move
		T take()
		{
			return protected_move(m_mutex, m_value);
		}

		/// Exchanges the value of the object with `u` and returns the old value
		template <typename U>
		T exchange(U&& u)
		{
			return protected_exchange(m_mutex, m_value, std::forward<U>(u));
		}

		/// Calls `func` with a const reference to the object which can be safely accessed; only available if the mutex is a shared mutex.
		template <typename FUNC>
		requires is_shared
		auto read_only_access(FUNC&& func) const
		{
			static_assert(std::is_invocable_v<FUNC, const_reference>, "Function must be invocable with a const reference to the object");
			return under_read_protection(m_mutex, func, m_value);
		}

		/// Sets a new value for the object
		template <typename U = T>
		void set(U&& value)
		{
			under_protection(m_mutex, [&]() { m_value = std::forward<U>(value); });
		}

		/// Calls `func` with a reference to the object which can be safely mutated
		template <typename FUNC>
		auto mutate_in_place(FUNC&& func)
		{
			static_assert(std::is_invocable_v<FUNC, reference>, "Function must be invocable with a reference to the object");
			return under_protection(m_mutex, func, m_value);
		}

		/// Calls `func` with a reference to a **copy** of the object, but not under the protection of the mutex. You can modify this copy safely.
		/// After `func` returns, the mutated copy will be set as the new value.
		template <typename FUNC>
		void mutate_by_copy(FUNC&& func)
		{
			static_assert(std::is_invocable_v<FUNC, reference>, "Function must be invocable with a reference to the object");
			auto copy = this->get();
			func(copy);
			this->set(std::move(copy));
		}

		mutex_type const& mutex() const { return m_mutex; }

	protected:

		T m_value;
		mutable mutex_type m_mutex;
	};

	/// Same as `protected_object` but uses std::shared_mutex by default.
	template <typename T, shared_mutexlike MUTEX_TYPE = std::shared_mutex>
	using shared_protected_object = protected_object<T, MUTEX_TYPE>;

	/// @}
}