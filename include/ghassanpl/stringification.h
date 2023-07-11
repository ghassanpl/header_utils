/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "string_ops.h"

namespace ghassanpl
{
	template <std::same_as<char> T>
	std::string to_string(T c) { return std::string{c}; }
	template <typename T>
	requires (!std::same_as<T, char> && std::integral<T>)
	std::string to_string(T val) { return std::to_string(val); }
	template <std::floating_point T>
	std::string to_string(T val)
	{
		const auto len = static_cast<size_t>(::snprintf(nullptr, 0, "%g", val));
		std::string str(len, '\0');
		::sprintf_s(&str[0], len+ 1, "%g", val);
		return str;
	}
	template <typename T>
	requires std::constructible_from<std::string_view, T>
	std::string to_string(T&& val) { return std::string{std::string_view{std::forward<T>(val)}}; }
	inline std::string to_string(nullptr_t) { return "null"; }
}

/// WIP WIP WIP WIP

namespace ghassanpl
{
	/// NOTE: The below is a MAJOR WORK IN PROGRESS

	template <bool TO_STRING>
	struct string_stringifier
	{
		std::string& result;

		template <typename... ARGS>
		bool operator()(ARGS&&... args)
		{
			((result += ghassanpl::to_string(args)), ...);
			return true;
		}
	};

	template <typename T>
	concept from_charsable = requires (const char* a, T& to) { { std::from_chars(a, a, to) } -> std::same_as<std::from_chars_result>; };

	template <typename T>
	bool eat_from_string(std::string_view& from, T&& val)
	{
		if constexpr (std::is_reference_v<T>)
		{
			if constexpr (std::same_as<T, char>)
			{
				if (from.empty()) return false;
				val = from[0];
				from.remove_prefix(1);
				return true;
			}
			else if constexpr (from_charsable<T>)
			{
				auto [ptr, ec] = std::from_chars(from.data(), from.data() + from.size(), val);
				if (ec == std::errc{})
				{
					from = ghassanpl::string_ops::make_sv(ptr, from.end());
					return true;
				}
				return false;
			}
			else
			{
				make_sv(from, val);
			}
		}
		else
		{
			if constexpr (std::same_as<T, char> || std::convertible_to<T, std::string_view>)
			{
				return ghassanpl::string_ops::consume(from, val);
			}
			else
			{
				make_sv(from, val);
			}
		}
	}

	template <>
	struct string_stringifier<false>
	{
		std::string_view from;

		template <typename... ARGS>
		bool operator()(ARGS&&... args)
		{
			return ((ghassanpl::eat_from_string(from, std::forward<ARGS>(args))) && ...);
		}
	};

	template <typename T, template<bool> typename STRINGIFIER = string_stringifier>
	requires requires (STRINGIFIER<true> str, T val) { stringify(str, val); }
	std::string to_string(T&& val)
	{
		std::string result;
		STRINGIFIER<true> str{result};
		stringify(str, std::forward<T>(val));
		return result;
	}

	template <typename T, template<bool> typename STRINGIFIER = string_stringifier>
	requires requires (STRINGIFIER<false> str, T val) { stringify(str, val); }
	bool from_string(std::string_view val, T& target)
	{
		STRINGIFIER<false> str{val};
		if (!stringify(str, target))
			return false;
		return true;
	}

	template <typename T, template<bool> typename STRINGIFIER = string_stringifier>
	requires requires (STRINGIFIER<false> str, T val) { stringify(str, val); }
	T from_string(std::string_view val)
	{
		T result{};
		STRINGIFIER<false> str{val};
		if (!stringify(str, result))
			return {};
		return result;
	}
}

template <typename T>
requires requires (ghassanpl::string_stringifier<true> str, T val) { stringify(str, val); }
struct std::formatter<T> : std::formatter<std::string>
{
	template<typename U, class FormatContext>
	auto format(U&& t, FormatContext& fc) const
	{
		return std::formatter<std::string>::format(to_string(std::forward<U>(t)), fc);
	}
};
