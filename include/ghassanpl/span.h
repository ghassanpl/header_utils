/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

namespace ghassanpl
{
	namespace xf {
		template <typename T> [[nodiscard]] constexpr auto as_span() noexcept { return [](auto const& val) { return std::span{ val }; }; }
	}

	template <typename T>
	[[nodiscard]] std::span<T> consume_n(std::span<T>& s, size_t n)
	{
		auto result = s.subspan(0, n);
		s = s.subspan(n);
		return result;
	}

	template <typename T>
	[[nodiscard]] decltype(auto) consume(std::span<T>& s)
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
