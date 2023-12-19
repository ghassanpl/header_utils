/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <string_view>
#include <set>

namespace ghassanpl
{
	/// TODO:
	/// Instead of tags, we need to have a struct that models the following concept:
	/// struct default_symbol_provider
	/// {
	/// 	using internal_value_type = std::string const*;
	///		using hash_type = size_t;
	/// 	static constexpr inline std::string empty_string;
	/// 	static internal_value_type empty_value() noexcept { return &empty_string; }
	///		static internal_value_type insert(std::string_view val)
	///		{
	///			auto& values = default_symbol_provider::values();
	///			if (auto v = values.find(val); v == values.end())
	///				return &*values.insert(std::string{ val }).first;
	///			else
	///				return &*v;
	///		}
	/// 	static std::string_view string_for(internal_value_type val) { return val ? std::string_view{*val} : std::string_view{}; }
	/// 	static hash_type hash_for(internal_value_type val) { return std::hash<const void*>{}(val); }
	/// 
	/// 	static std::set<std::string, std::less<>>& values() noexcept
	/// 	{
	/// 		static std::set<std::string, std::less<>> values;
	/// 		return values;
	/// 	}
	/// 
	/// };

	/// template <typename T>
	/// concept symbol_provider = std::is_class_v<T> && requires { typename T::internal_value_type;  };

	template <typename TAG>
	struct namespaced_symbol
	{
		static constexpr inline std::string empty_string;
		std::string const* value = &empty_string;

		explicit namespaced_symbol(std::string_view val) : value{ val.empty() ? &empty_string : insert(val) } { }
		namespaced_symbol() noexcept = default;
		namespaced_symbol(namespaced_symbol const&) noexcept = default;
		namespaced_symbol(namespaced_symbol&&) noexcept = default;
		namespaced_symbol& operator=(namespaced_symbol const&) noexcept = default;
		namespaced_symbol& operator=(namespaced_symbol&&) noexcept = default;

		static std::set<std::string, std::less<>>& values() noexcept
		{
			static std::set<std::string, std::less<>> values;
			return values;
		}

		static std::string const* insert(std::string_view val)
		{
			auto& values = namespaced_symbol<TAG>::values();
			if (auto v = values.find(val); v == values.end())
				return &*values.insert(std::string{ val }).first;
			else
				return &*v;
		}

		explicit operator std::string_view() const noexcept { return *value; }

		std::string const* operator->() const noexcept { return value; }

		bool operator==(namespaced_symbol const& other) const noexcept { return value == other.value; }
		auto operator<=>(namespaced_symbol const& other) const noexcept { return value <=> other.value; }

		friend bool operator==(std::string_view a, namespaced_symbol const& b) noexcept { return a == *b.value; }
		friend auto operator<=>(std::string_view a, namespaced_symbol const& b) noexcept { return a <=> *b.value; }
	};

	using symbol = namespaced_symbol<void>;

	template <typename TAG = void>
	std::string_view symbol_for(std::string_view val)
	{
		auto& sym_values = namespaced_symbol<TAG>::values();
		const auto v = sym_values.find(val);
		return (v == sym_values.end()) ? **sym_values.insert(std::string{ val }).first : **v;
	}

}

/// TODO: ostream << and formatter, or enable stringification

template <> struct std::hash<ghassanpl::symbol> { size_t operator()(const ghassanpl::symbol& x) const noexcept { return std::hash<void const*>{}(x.value); } };
