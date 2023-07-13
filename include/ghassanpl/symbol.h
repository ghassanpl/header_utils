/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <string_view>
#include <set>

namespace ghassanpl
{
	template <typename TAG>
	struct namespaced_symbol
	{
		std::string_view value{};

		explicit namespaced_symbol(std::string_view val) : value{ val.empty() ? std::string_view{} : insert(val) } { }
		namespaced_symbol() noexcept = default;
		namespaced_symbol(namespaced_symbol const&) noexcept = default;
		namespaced_symbol(namespaced_symbol&&) noexcept = default;
		namespaced_symbol& operator=(namespaced_symbol const&) noexcept = default;
		namespaced_symbol& operator=(namespaced_symbol&&) noexcept = default;

		static auto& values() noexcept
		{
			static std::set<std::string, std::less<>> values;
			return values;
		}

		static std::string_view insert(std::string_view val)
		{
			auto& values = namespaced_symbol<TAG>::values();
			if (auto v = values.find(val); v == values.end())
				return *values.insert(std::string{ val }).first;
			else
				return *v;
		}

		explicit operator std::string_view() const noexcept { return value; }

		bool operator==(namespaced_symbol const& other) const noexcept { return value.data() == other.value.data(); }
		auto operator<=>(namespaced_symbol const& other) const noexcept { return value <=> other.value; }

		friend bool operator==(std::string_view a, namespaced_symbol const& b) noexcept { return a == b.value; }
		friend auto operator<=>(std::string_view a, namespaced_symbol const& b) noexcept { return a <=> b.value; }
	};

	using symbol = namespaced_symbol<void>;

	template <typename TAG = void>
	std::string_view symbol_for(std::string_view val)
	{
		auto& sym_values = namespaced_symbol<TAG>::values();
		const auto v = sym_values.find(val);
		return (v == sym_values.end()) ? *sym_values.insert(std::string{ val }).first : *v;
	}

}

/// TODO: ostream << and formatter, or enable stringification

template <> struct std::hash<ghassanpl::symbol> { size_t operator()(const ghassanpl::symbol& x) const noexcept { return std::hash<std::string_view>{}(x.value); } };
