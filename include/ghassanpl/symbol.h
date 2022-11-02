/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <string>
#include <string_view>
#include <set>

namespace ghassanpl
{
	struct symbol
	{
		std::string_view value{};

		explicit symbol(std::string_view val) : value{ insert(val) } { }
		symbol() noexcept = default;
		symbol(symbol const&) noexcept = default;
		symbol(symbol&&) noexcept = default;
		symbol& operator=(symbol const&) noexcept = default;
		symbol& operator=(symbol&&) noexcept = default;

		static auto& values() noexcept
		{
			static std::set<std::string, std::less<>> values;
			return values;
		}

		static std::string_view insert(std::string_view val)
		{
			auto& values = symbol::values();
			if (auto v = values.find(val); v == values.end())
				return *values.insert(std::string{ val }).first;
			else
				return *v;
		}

		explicit operator std::string_view() const noexcept { return value; }

		bool operator==(symbol const& other) const noexcept { return value.data() == other.value.data(); }
		auto operator<=>(symbol const& other) const noexcept { return value <=> other.value; }

		friend bool operator==(std::string_view a, symbol const& b) noexcept { return a == b.value; }
		friend auto operator<=>(std::string_view a, symbol const& b) noexcept { return a <=> b.value; }
	};

	std::string_view symbol_for(std::string_view val)
	{
		auto& sym_values = symbol::values();
		const auto v = sym_values.find(val);
		return (v == sym_values.end()) ? *sym_values.insert(std::string{ val }).first : *v;
	}

}

/// TODO: ostream << and formatter

namespace std
{
	template <> struct hash<ghassanpl::symbol> { size_t operator()(const ghassanpl::symbol& x) const { return std::hash<std::string_view>{}(x.value); } };
}
