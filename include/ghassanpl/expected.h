/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <system_error>
#include <optional>
#define GHPL_LATEST_MSVC_VERSION_WITH_EXPECTED_BUGS 1939 
#if __has_include(<expected>) && (_MSC_VER > GHPL_LATEST_MSVC_VERSION_WITH_EXPECTED_BUGS)
#include <expected>
namespace ghassanpl
{
	using std::expected;
	using std::unexpected;
}
#elif __has_include(<tl/expected.hpp>)
#include <tl/expected.hpp>
namespace ghassanpl
{
	using tl::expected;
	using tl::unexpected;
}
#elif defined(GHPL_EXPECTED_HEADER)
#include GHPL_EXPECTED_HEADER
#else
#error "No expected implementation found"
#endif

namespace ghassanpl
{

	/// Calls the given function with `args` and an `std::error_code` as the last argument
	/// \returns an `expected` with the result of the function call if the `std::error_code` is `0`, otherwise an `unexpected` with the error code
	template <typename FUNC, typename... ARGS>
	inline auto call_with_expected_ec(FUNC&& func, ARGS&&... args) noexcept(noexcept(func(std::forward<ARGS>(args)...)))
		-> expected<std::invoke_result_t<FUNC, ARGS&&...>, std::error_code>
	{
		std::error_code ec{};
		if constexpr (std::is_void_v<std::invoke_result_t<FUNC, ARGS&&...>>)
		{
			func(std::forward<ARGS>(args)..., ec);
			if (ec)
				return unexpected(ec);
			return {};
		}
		else
		{
			auto result = func(std::forward<ARGS>(args)..., ec);
			if (ec)
				return unexpected(ec);
			return result;
		}
	}
	
	template <typename T>
	struct undroppable final
	{
		template <typename... ARGS, typename = std::enable_if_t<std::is_constructible_v<T, ARGS...>>>
		undroppable(ARGS&&... args) noexcept(std::is_nothrow_constructible_v<T, ARGS...>)
			: m_value(std::forward<ARGS>(args)...)
		{
		}
		
		undroppable(undroppable const& other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		undroppable& operator=(undroppable const& other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		undroppable(undroppable&& other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		undroppable& operator=(undroppable&& other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		T handle() noexcept
		{
			return std::exchange(m_value, {}).value();
		}

		bool was_handled() const noexcept { return !m_value.has_value(); }

		T const& value() const noexcept { return m_value.value(); }
		T& value() noexcept { return m_value.value(); }

		~undroppable() noexcept(false)
		{
			if (m_value)
				throw std::move(m_value.value());
		}

	private:

		std::optional<T> m_value;
	};

#define or_return(exp) if (decltype(auto) _return_val = (exp); !_return_val.has_value()) return _return_val
#define or_break(exp) if (decltype(auto) _return_val = (exp); !_return_val.has_value()) break
}
