/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include "flat_map.h"
#include <functional>
#include <utility>

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

		mutlticast_function(mutlticast_function const&) noexcept = default;
		mutlticast_function(mutlticast_function&&) noexcept = default;
		mutlticast_function& operator =(mutlticast_function const&) noexcept = default;
		mutlticast_function& operator =(mutlticast_function&&) noexcept = default;

		template <typename... F>
		requires (std::is_invocable_r_v<R, F, ARGS...> && ...)
		explicit(false) mutlticast_function(F&&... listeners)
		{
			(this->add(std::function<F>(listeners)), ...);
		}

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
		/// \todo Adding/removing callbacks during the calls is unsafe...
		template <typename... CALL_ARGS>
		auto operator()(CALL_ARGS&&... args) const
		{
			return call_helper<R>::call(m_listeners, std::forward<CALL_ARGS>(args)...);
		}

		/// Removes all the invocables from this objects
		void clear()
		{
			m_listeners.clear();
		}

		/// Returns a view over all the added invocables
		auto const& listeners() const { return m_listeners; }

	private:

		template <typename RET>
		struct call_helper
		{
			template <typename... CALL_ARGS>
			static std::vector<RET> call(flat_map_preferred<handle, std::function<RET(ARGS...)>> const& listeners, CALL_ARGS&&... args)
			{
				std::vector<RET> ret;
				ret.reserve(listeners.size());
				for (auto&& [handle, listener] : listeners)
					ret.push_back(listener(args...));
				return ret;
			}
		};

		template <>
		struct call_helper<void>
		{
			template <typename... CALL_ARGS>
			static void call(flat_map_preferred<handle, std::function<void(ARGS...)>> const& listeners, CALL_ARGS&&... args)
			{
				for (auto&& [handle, listener] : listeners)
					listener(args...);
			}
		};

		flat_map_preferred<handle, std::function<R(ARGS...)>> m_listeners;
		size_t m_last_id = {};

		/// TODO:
		//struct call_temps
		//{
		//	std::vector<std::pair< handle, std::function<R(ARGS...)>>> to_add;
		//	std::vector<handle> to_remove;
		//};
		//call_temps* during_call = nullptr;
	};
}