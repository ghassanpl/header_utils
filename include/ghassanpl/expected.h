/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#define GHPL_LATEST_MSVC_VERSION_WITH_EXPECTED_BUGS 1939 
#if __has_include(<expected>) && (_MSC_VER > GHPL_LATEST_MSVC_VERSION_WITH_EXPECTED_BUGS)
#include <expected>
namespace ghassanpl
{
	using std::expected;
	using std::unexpected;

	/// Hacks to make expected work with MSVC
	static_assert(std::is_swappable_v<std::string>);
	static_assert(std::is_swappable_v<std::error_code>);
}
#elif __has_include(<tl/expected.hpp>)
#include <tl/expected.hpp> /// TODO: Until MSVC fixes its expected impl
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

}
