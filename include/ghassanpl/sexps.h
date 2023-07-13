/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "string_ops.h"
#include "parsing.h"
#include <nlohmann/json.hpp>

namespace ghassanpl::formats::sexpressions
{
	/// Consumes `[list]` or `atom`
	auto consume_value(std::string_view& sexp_str, std::array<char, 2> braces = { '[', ']' }) -> nlohmann::json;

	/// Consumes a space-delimited `word`, `'string' "literal"`, or `14.46` number
	auto consume_atom(std::string_view& sexp_str, char end_brace = ']') -> nlohmann::json;

	/// Consumes space-delimited elements until close-brace or end-of-string.
	/// `sexp_str` MUST NOT start with open-brace.
	auto consume_list(std::string_view& sexp_str, std::array<char, 2> braces = { '[', ']' }) -> nlohmann::json;

	inline auto parse_value(std::string_view sexp_str, std::array<char, 2> braces = { '[', ']' }) -> nlohmann::json { return consume_value(sexp_str, braces); }
	inline auto parse_atom(std::string_view sexp_str, char end_brace = ']') -> nlohmann::json { return consume_atom(sexp_str, end_brace); }
	inline auto parse_list(std::string_view sexp_str, std::array<char, 2> braces = { '[', ']' }) -> nlohmann::json { return consume_list(sexp_str, braces); }
}

/// Implementation

namespace ghassanpl::formats::sexpressions
{
	inline auto consume_atom(std::string_view& sexp_str, char end_brace) -> nlohmann::json
	{
		string_ops::trim_whitespace_left(sexp_str);

		/// Try string literals first
		if (sexp_str.starts_with('\''))
			return parsing::consume_c_string<'\''>(sexp_str).second;
		else if (sexp_str.starts_with('"'))
			return parsing::consume_c_string<'"'>(sexp_str).second;

		auto result = string_ops::consume_until(sexp_str, [=](auto ch) { return string_ops::ascii::isspace(ch) || ch == end_brace; });

		/// Try paring as number
		double num_result;
		const auto fcres = string_ops::from_chars(result, num_result);
		if (fcres.ec == std::errc{} && fcres.ptr == sexp_str.data()) /// If we ate the ENTIRE number
		{
			string_ops::trim_whitespace_left(sexp_str);
			return num_result;
		}

		string_ops::trim_whitespace_left(sexp_str);
		return result;
	}

	inline auto consume_list(std::string_view& sexp_str, std::array<char, 2> braces) -> nlohmann::json
	{
		nlohmann::json result = nlohmann::json::array();
		string_ops::trim_whitespace_left(sexp_str);
		while (!sexp_str.empty() && !sexp_str.starts_with(braces[1]))
		{
			result.push_back(consume_value(sexp_str, braces));
			string_ops::trim_whitespace_left(sexp_str);
		}
		std::ignore = string_ops::consume(sexp_str, braces[1]);
		return result;
	}

	inline auto consume_value(std::string_view& sexp_str, std::array<char, 2> braces) -> nlohmann::json
	{
		string_ops::trim_whitespace_left(sexp_str);
		if (string_ops::consume(sexp_str, braces[0]))
			return consume_list(sexp_str, braces);
		return consume_atom(sexp_str, braces[1]);
	}

}