/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#if defined(__cpp_lib_span)
#include <span>
namespace ghassanpl
{
	using std::span;
}
#elif __has_include(<gsl/span>)
#include <gsl/span>
namespace ghassanpl
{
	using gsl::span;
}
#elif __has_include(<gsl/gsl-lite.hpp>)
#include <gsl/gsl-lite.hpp>
namespace ghassanpl
{
	using std20::span;
}
#elif defined(GHPL_SPAN_HEADER)
#include GHPL_SPAN_HEADER
#else
#error "No span implementation found"
#endif

namespace ghassanpl
{
	namespace xf {
		template <typename T> [[nodiscard]] constexpr auto as_span() noexcept { return [](auto const& val) { return ghassanpl::span{ val }; }; }
	}

	template <typename T>
	[[nodiscard]] span<T> consume_n(span<T>& s, size_t n)
	{
		auto result = s.subspan(0, n);
		s = s.subspan(n);
		return result;
	}

	template <typename T>
	[[nodiscard]] decltype(auto) consume(span<T>& s)
	{
		if constexpr (std::is_const_v<T>) {
			auto const& ref = s[0];
			s = s.subspan(1);
			return ref;
		}
		else {
			auto val = std::move(s[0]);
			s = s.subspan(1);
			return val;
		}
	}
}
