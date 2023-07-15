/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "eval.h"
#include "sexps.h"
#include "json_helpers.h"

namespace ghassanpl
{
	template <typename FUNC>
	std::string interpolate_simple(std::string_view str, FUNC&& func)
	{
		static_assert(std::invocable<FUNC, std::string_view> && std::constructible_from<std::string, std::invoke_result_t<FUNC, std::string_view>>, "function must take a stringable and return a string");
		std::string result;
		while (!str.empty())
		{
			result += string_ops::consume_until_delim(str, '[');
			if (str.empty()) break;
			if (string_ops::consume(str, '['))
				result += '[';
			else
			{
				auto key = string_ops::consume_until_delim(str, ']');
				result += func(key);
			}
		}
		return result;
	}

	template <bool SYNTAX>
	std::string interpolate_eval(std::string_view str, eval_env<SYNTAX>& env)
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
				using value = eval_env<SYNTAX>::value;
				value call = formats::sexpressions::consume_list(str);
				value call_result = env.eval(call);
				formats::json::visit(env.ref(call_result), [&](auto&& val) {
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

	/// https://projectfluent.org/ <- a nice example of what we could implement with sexps interpolate
}