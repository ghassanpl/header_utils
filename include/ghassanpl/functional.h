/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <functional>
#include <optional>
#include <utility>
#include <map>

// Shamelessly stolen from https://github.com/klmr/multifunction
namespace ghassanpl
{
	/// \addtogroup Functional
	/// Additional useful delegates (like `std::function<>`)

	template <typename R, typename... ARGS>
	struct multicast_function_traits
	{
		using return_type = R;
		using argument_types = std::tuple<ARGS...>;
	};

	template <typename R, typename ARG>
	struct multicast_function_traits<R, ARG>
	{
		using return_type = R;
		using argument_type = ARG;
	};

	template <typename R, typename ARG1, typename ARG2>
	struct multicast_function_traits<R, ARG1, ARG2>
	{
		using return_type = R;
		using first_argument_type = ARG1;
		using second_argument_type = ARG2;
	};


	template <typename R, typename... ARGS>
	class mutlticast_function;
	
	/// Like `std::function<R(ARGS...)>` but can reference multiple invocables, and call them all at once
	/// \ingroup Functional
	template <typename R, typename... ARGS>
	class mutlticast_function<R(ARGS...)> : public multicast_function_traits<R, ARGS...>
	{
	public:

		enum class handle : size_t {};

		mutlticast_function() noexcept = default;
		mutlticast_function(mutlticast_function const&) noexcept = default;
		mutlticast_function(mutlticast_function&&) noexcept = default;
		mutlticast_function& operator =(mutlticast_function const&) noexcept = default;
		mutlticast_function& operator =(mutlticast_function&&) noexcept = default;

		/// Adds a new invocable to the list
		/// \returns A handle that can be used to remove the added invocable
		template <typename F>
		handle operator+=(F&& listener) { return add(std::forward<F>(listener)); }

		/// Adds a new invocable to the list
		/// \returns A handle that can be used to remove the added invocable
		template <typename F>
		handle add(F&& listener)
		{
			const auto new_id = handle{ m_last_id++ };
			m_listeners.emplace(new_id, std::forward<F>(listener));
			return new_id;
		}

		/// Removes the invocable associated with `handle`
		void operator-=(handle handle) { remove(handle); }

		/// Removes the invocable associated with `handle`
		void remove(handle handle)
		{
			m_listeners.erase(handle);
		}

		/// Calls all the invocables added to this object
		///
		/// \returns If the return value is void, returns void, otherwise returns a vector of all the return values of the added invocables
		template <typename... CALL_ARGS>
		auto operator()(CALL_ARGS&&... args) const
		{
			return call_helper<R>::template call(m_listeners, std::forward<CALL_ARGS>(args)...);
		}

		/// Removes all the invocables from this objects
		void clear()
		{
			m_listeners.clear();
		}

		/// Returns a view over all the added invocables
		auto const& listeners() const { return m_listeners; }

	private:

		template <typename R>
		struct call_helper
		{
			template <typename... CALL_ARGS>
			static std::vector<R> call(std::map<handle, std::function<R(ARGS...)>> const& listeners, CALL_ARGS&&... args)
			{
				std::vector<R> ret;
				ret.reserve(listeners.size());
				for (auto& [handle, listener] : listeners)
					ret.push_back(listener(std::forward<CALL_ARGS>(args)...));
				return ret;
			}
		};

		template <>
		struct call_helper<void>
		{
			template <typename... CALL_ARGS>
			static void call(std::map<handle, std::function<void(ARGS...)>> const& listeners, CALL_ARGS&&... args)
			{
				for (auto& [handle, listener] : listeners)
					listener(std::forward<CALL_ARGS>(args)...);
			}
		};

		std::map<handle, std::function<R(ARGS...)>> m_listeners;
		size_t m_last_id = {};
	};

	/// Returns a function that calls `func` when invoked, but only the first time
	/// \ingroup Functional
	template <typename... ARGS>
	[[nodiscard]] auto make_single_time_function(std::function<void(ARGS...)> func)
	{
		return [func = std::move(func)]<typename... CALL_ARGS>(CALL_ARGS&&... args) mutable {
			if (func) std::exchange(func, {})(std::forward<CALL_ARGS>(args)...);
		};
	}

	/// Returns a function that calls `func` when invoked, but only the first time
	/// \ingroup Functional
	template <typename FUNC>
	[[nodiscard]] auto make_single_time_function(FUNC&& func)
	{
		return make_single_time_function(std::function{ std::forward<FUNC>(func) });
	}

	///
	template <typename T, typename FUNC>
	[[nodiscard]] auto transform(std::optional<T> const& value, FUNC&& func) -> decltype(std::optional{ func(value.value()) })
	{
		return value ? std::optional{ func(value.value()) } : std::nullopt;
	}

} // namespace util
