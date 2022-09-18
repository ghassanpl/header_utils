/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "string_ops.h"
#include <optional>

namespace ghassanpl::parsing
{

	struct parse_error : std::runtime_error
	{
		std::string_view Where;

		template <GHPL_FORMAT_TEMPLATE>
		parse_error(std::string_view where, GHPL_FORMAT_ARGS)
			: runtime_error(GHPL_FORMAT_CALL)
			, Where(where)
		{

		}
	};

	bool try_eat(std::string_view& str, std::string_view what)
	{
		string_ops::trim_whitespace_left(str);
		if (!str.starts_with(what))
			return false;
		str.remove_prefix(what.size());
		return true;
	}

	bool try_eat(std::string_view& str, char what)
	{
		string_ops::trim_whitespace_left(str);
		if (!str.starts_with(what))
			return false;
		str.remove_prefix(1);
		return true;
	}

	void eat(std::string_view& str, std::string_view what)
	{
		if (!try_eat(str, what))
			throw parse_error(str, "expected '{}'", what);
	}

	void eat(std::string_view& str, char what)
	{
		if (!try_eat(str, what))
			throw parse_error(str, "expected '{}'", what);
	}

	std::string_view try_eat_identifier(std::string_view& str)
	{
		string_ops::trim_whitespace_left(str);
		return string_ops::consume_c_identifier(str);
	}

	std::string_view eat_identifier(std::string_view& str)
	{
		auto result = try_eat_identifier(str);
		if (result.empty())
			throw parse_error(str, "expected identifier");
		return result;
	}

	std::string_view eat_whitespace(std::string_view& str)
	{
		return string_ops::consume_while(str, string_ops::ascii::isspace);
	}

	bool try_eat_line_comment(std::string_view& str, std::string_view comment_start = "//")
	{
		string_ops::trim_whitespace_left(str);
		if (!try_eat(str, comment_start))
			return false;
		std::ignore = string_ops::consume_until(str, '\n');
		return true;
	}

	bool try_eat_unsigned(std::string_view& str, uint64_t& result, int base = 10)
	{
		string_ops::trim_whitespace_left(str);
		auto [parsed, value] = string_ops::consume_c_unsigned(str, base);
		if (parsed.empty()) return false;
		result = value;
		return true;
	}

	std::optional<uint64_t> try_eat_unsigned(std::string_view& str, int base = 10)
	{
		uint64_t result{};
		if (try_eat_unsigned(str, result, base))
			return result;
		return std::nullopt;
	}

	bool try_eat_integer(std::string_view& str, int64_t& result, int base = 10)
	{
		string_ops::trim_whitespace_left(str);
		auto [parsed, value] = string_ops::consume_c_integer(str, base);
		if (parsed.empty()) return false;
		result = value;
		return true;
	}

	std::optional<int64_t> try_eat_integer(std::string_view& str, int base = 10)
	{
		int64_t result{};
		if (try_eat_integer(str, result, base))
			return result;
		return std::nullopt;
	}

	uint64_t eat_unsigned(std::string_view& str, int base = 10)
	{
		uint64_t result{};
		if (!try_eat_unsigned(str, result, base))
			throw parse_error(str, "expected unsigned integer of base {}", base);
		return result;
	}

	int64_t eat_integer(std::string_view& str, int base = 10)
	{
		int64_t result{};
		if (!try_eat_integer(str, result, base))
			throw parse_error(str, "expected integer of base {}", base);
		return result;
	}

	char32_t try_eat_utf8_codepoint(std::string_view& str)
	{
		return string_ops::consume_utf8(str);
	}

	char32_t eat_utf8_codepoint(std::string_view& str)
	{
		if (auto cp = string_ops::consume_utf8(str))
			return cp;
		throw parse_error(str, "expected UTF-8 codepoint");
	}
}