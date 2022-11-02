/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "unicode.h"
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

	inline bool try_eat(std::string_view& str, std::string_view what)
	{
		string_ops::trim_whitespace_left(str);
		if (!str.starts_with(what))
			return false;
		str.remove_prefix(what.size());
		return true;
	}

	inline bool try_eat(std::string_view& str, char what)
	{
		string_ops::trim_whitespace_left(str);
		if (!str.starts_with(what))
			return false;
		str.remove_prefix(1);
		return true;
	}

	inline void eat(std::string_view& str, std::string_view what)
	{
		if (!try_eat(str, what))
			throw parse_error(str, "expected '{}'", what);
	}

	inline void eat(std::string_view& str, char what)
	{
		if (!try_eat(str, what))
			throw parse_error(str, "expected '{}'", what);
	}

	inline std::string_view try_eat_identifier(std::string_view& str)
	{
		string_ops::trim_whitespace_left(str);
		return string_ops::consume_c_identifier(str);
	}

	inline std::string_view eat_identifier(std::string_view& str)
	{
		auto result = try_eat_identifier(str);
		if (result.empty())
			throw parse_error(str, "expected identifier");
		return result;
	}

	inline std::string_view try_eat_identifier_with(std::string_view& str, std::string_view additional_chars)
	{
		string_ops::trim_whitespace_left(str);
		return string_ops::consume_c_identifier_with(str, additional_chars);
	}

	inline std::string_view eat_identifier_with(std::string_view& str, std::string_view additional_chars)
	{
		auto result = try_eat_identifier_with(str, additional_chars);
		if (result.empty())
			throw parse_error(str, "expected identifier");
		return result;
	}

	inline std::string_view eat_whitespace(std::string_view& str)
	{
		return string_ops::consume_while(str, string_ops::ascii::isspace);
	}

	inline bool try_eat_line_comment(std::string_view& str, std::string_view comment_start = "//")
	{
		string_ops::trim_whitespace_left(str);
		if (!try_eat(str, comment_start))
			return false;
		std::ignore = string_ops::consume_until(str, '\n');
		return true;
	}

	inline bool try_eat_unsigned(std::string_view& str, uint64_t& result, int base = 10)
	{
		string_ops::trim_whitespace_left(str);
		auto [parsed, value] = string_ops::consume_c_unsigned(str, base);
		if (parsed.empty()) return false;
		result = value;
		return true;
	}

	inline std::optional<uint64_t> try_eat_unsigned(std::string_view& str, int base = 10)
	{
		uint64_t result{};
		if (try_eat_unsigned(str, result, base))
			return result;
		return std::nullopt;
	}

	inline bool try_eat_integer(std::string_view& str, int64_t& result, int base = 10)
	{
		string_ops::trim_whitespace_left(str);
		auto [parsed, value] = string_ops::consume_c_integer(str, base);
		if (parsed.empty()) return false;
		result = value;
		return true;
	}

	inline std::optional<int64_t> try_eat_integer(std::string_view& str, int base = 10)
	{
		int64_t result{};
		if (try_eat_integer(str, result, base))
			return result;
		return std::nullopt;
	}

	inline uint64_t eat_unsigned(std::string_view& str, int base = 10)
	{
		uint64_t result{};
		if (!try_eat_unsigned(str, result, base))
			throw parse_error(str, "expected unsigned integer of base {}", base);
		return result;
	}

	inline int64_t eat_integer(std::string_view& str, int base = 10)
	{
		int64_t result{};
		if (!try_eat_integer(str, result, base))
			throw parse_error(str, "expected integer of base {}", base);
		return result;
	}

	inline char32_t try_eat_utf8_codepoint(std::string_view& str)
	{
		return string_ops::consume_utf8(str);
	}

	inline char32_t eat_utf8_codepoint(std::string_view& str)
	{
		if (auto cp = string_ops::consume_utf8(str))
			return cp;
		throw parse_error(str, "expected UTF-8 codepoint");
	}

#if 0
	namespace decade
	{
		template <GHPL_FORMAT_TEMPLATE>
		[[nodiscard]] inline parsing::parse_error parse_error(token_range const& range, GHPL_FORMAT_ARGS) { return parsing::parse_error(to_string_view(range), GHPL_FORMAT_FORWARD); }
		template <GHPL_FORMAT_TEMPLATE>
		[[nodiscard]] inline parsing::parse_error parse_error(token_it const& it, GHPL_FORMAT_ARGS) { return parsing::parse_error(it->range, GHPL_FORMAT_FORWARD); }

		struct expression
		{
			virtual ~expression() noexcept = default;
			token_range source_range;

			expression(token_it it) : source_range(it, std::next(it)) {}
			expression(token_range range) : source_range(range) {}
		};

		struct function_call_expression : public expression
		{
			std::vector<std::unique_ptr<expression>> arguments;
			std::string name;
			static std::string make_name(std::span<token_it const> name_parts, bool infix)
			{
				std::string name;
				if (infix)
					name += ':';
				for (token_it it : name_parts)
				{
					name += it->range;
					name += ':';
				}
				return name;
			}
			function_call_expression(token_range range, std::span<token_it const> name_, std::vector<std::unique_ptr<expression>> arguments_, bool infix) 
				: expression(range)
				, name(make_name(name_, infix))
				, arguments(std::move(arguments_))
			{
			}
		};

		struct identifier_expression : public expression
		{
			std::string identifier;

			identifier_expression(token_it it) : expression(it), identifier(it->range) {}
		};

		struct literal_expression : public expression
		{
			token literal;

			literal_expression(token_it it) : expression(it), literal(*it) {}
		};

		inline std::unique_ptr<expression> parse_expression(token_range& tokens)
		{
			std::unique_ptr<expression> result;

			auto start = tokens.begin();

			std::vector<std::unique_ptr<expression>> constituents;
			while (tokens && tokens.front().type >= token::word)
			{
				if (tokens.front().type == token::word)
				{
					constituents.push_back(std::make_unique<identifier_expression>(tokens.begin()));
					tokens.advance(1);
				}
				else if (tokens.front().type == token::number || tokens.front().type == token::string)
				{
					constituents.push_back(std::make_unique<literal_expression>(tokens.begin()));
					tokens.advance(1);
				}
				else if (tokens.front().type == token::start_sub_expression)
				{
					tokens.advance(1);
					constituents.push_back(parse_expression(tokens));

					if (tokens.front().type != token::end_sub_expression)
						throw parse_error(tokens.begin(), "unexpected end of line");
					tokens.advance(1);
				}
				else
					throw parse_error(tokens.begin(), "expected expression part");
			}
			if (constituents.empty())
				throw parse_error(tokens.begin(), "empty expression encountered");

			const auto constitutent_count = constituents.size();
			if (constitutent_count == 1)
				return std::move(constituents[0]);

			const bool infix = (constitutent_count % 2) == 1;

			std::vector<token_it> function_name;
			std::vector<std::unique_ptr<expression>> arguments;
			if (infix)
				arguments.push_back(std::exchange(constituents[0], {}));
			/// First, check that all function words are actually words
			for (size_t i = infix; i < constitutent_count; i += 2)
			{
				auto& function_identifier = constituents[i];
				if (auto identifier = dynamic_cast<identifier_expression*>(function_identifier.get()))
				{
					function_name.push_back(identifier->source_range.begin());
					arguments.push_back(std::exchange(constituents[i + 1], {}));
				}
				else
					throw parse_error(function_identifier->source_range, "expected function name part");
			}

			return std::make_unique<function_call_expression>(std::ranges::subrange(start, tokens.begin()), std::move(function_name), std::move(arguments), infix);
		}

		inline std::unique_ptr<expression> parse_expression(std::string_view& str)
		{
			const auto tokens = lex(str);
			using tokenit = std::ranges::iterator_t<decltype(tokens)>;
			token_range range = tokens;
			return parse_expression(range);
		}

	}
#endif
}