/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "eval.h"
#include "sexps.h"
#include "json_helpers.h"

namespace ghassanpl
{
	/// \defgroup StringInterpolate String Interpolation
	/// TODO: Documentation
	/// @{

	template <char open = '[', char close = ']', typename FUNC>
	[[nodiscard]] std::string interpolate_simple(std::string_view str, FUNC&& func)
	{
		static_assert(std::invocable<FUNC, std::string_view> && std::constructible_from<std::string, std::invoke_result_t<FUNC, std::string_view>>, "function must take a stringable and return a string");
		std::string result;
		while (!str.empty())
		{
			result += string_ops::consume_until_delim_ex(str, open);
			if (str.empty()) break;
			if (string_ops::consume(str, open))
				result += open;
			else
			{
				auto key = string_ops::consume_until_delim_ex(str, close);
				result += func(key);
			}
		}
		return result;
	}

	template <char open = '[', char close = ']', typename FUNC>
	[[nodiscard]] std::string interpolate_recursive(std::string_view str, FUNC&& func)
	{
		static_assert(std::invocable<FUNC, std::string_view> && std::constructible_from<std::string, std::invoke_result_t<FUNC, std::string_view>>, "function must take a stringable and return a string");
		std::string result;
		while (!str.empty())
		{
			result += string_ops::consume_until_delim_ex(str, open);
			if (str.empty()) break;
			if (string_ops::consume(str, open))
				result += open;
			else
			{
				size_t opened = 0;
				auto key = str;
				while (!str.empty() && (str[0] != close || opened))
				{
					if (str[0] == close && opened)
						opened--;
					else if (str[0] == open)
						opened++;
					str.remove_prefix(1);
				}
				key = { key.data(), str.data() };
				std::ignore = string_ops::consume(str, close);

				result += func(interpolate_recursive<open, close>(key, func));
			}
		}
		return result;
	}

	template <bool SYNTAX>
	[[nodiscard]] std::string interpolate_eval(std::string_view str, eval::environment<SYNTAX>& env)
	{
		std::string result;
		while (!str.empty())
		{
			result += string_ops::consume_until(str, '[');
			if (str.empty()) break;
			str.remove_prefix(1);
			if (string_ops::consume(str, '['))
				result += '[';
			else
			{
				using eval::value;
				value call = formats::sexpressions::consume_list(str);
				value call_result = env.eval(call);
				formats::json::visit(call_result, [&](auto&& val) {
					using std::to_string;
					using nlohmann::to_string;
					using ghassanpl::string_ops::to_string;
					if constexpr (requires { { to_string(val) }; })
						result += to_string(val);
				});
			}
		}
		return result;
	}

	/// @}
	
	/// https://projectfluent.org/ <- a nice example of what we could implement with sexps interpolate
}