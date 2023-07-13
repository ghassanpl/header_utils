/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "parsing.h"
#include "mmap.h"
#include "json_helpers.h"
#include <ostream>

/// TODO: https://dev.stenway.com/SML/Examples.html

namespace ghassanpl::formats::wilson
{
	auto parse(std::string_view wilson_str) -> nlohmann::json;
	auto parse_object(std::string_view wilson_str, char closing_char = '}') -> nlohmann::json;
	auto parse_array(std::string_view wilson_str) -> nlohmann::json;
	/// Parses: a word as a string value or bool/null value; a "string literal" as a string value
	auto parse_word_or_string(std::string_view wilson_str) -> nlohmann::json;
	/// Parses a word or a string literal a string value
	auto parse_string_value(std::string_view wilson_str) -> std::string;
	/// Parses a string literal as a string value
	auto parse_string_literal(std::string_view wilson_str) -> std::string;

	auto consume_string_literal(std::string_view& _str) -> std::string;
	auto consume_string_value(std::string_view& str) -> std::string;
	auto consume_word_or_string(std::string_view& str) -> nlohmann::json;
	auto consume_object(std::string_view& wilson_str, char closing_char = '}') -> nlohmann::json;
	auto consume_array(std::string_view& wilson_str, char closing_char = ']') -> nlohmann::json;
	auto consume_value(std::string_view& wilson_str) -> nlohmann::json;

	template <typename OUTFUNC>
	void output(OUTFUNC&& out, nlohmann::json const& value);

	inline void output_to_stream(std::ostream& strm, nlohmann::json const& value)
	{
		output([&](std::string_view val) { strm.write(val.data(), val.size()); }, value);
	}

	std::string to_string(nlohmann::json const& value);

	nlohmann::json load_file(std::filesystem::path const& from, std::error_code& ec);
	nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json const& or_json = json::empty_json);
}

namespace ghassanpl::formats::wilson
{
	inline nlohmann::json consume_array(std::string_view& str, char closing_char)
	{
		auto obj = nlohmann::json::array();
		do
		{
			string_ops::trim_whitespace_left(str);
			if (string_ops::consume(str, closing_char))
				return obj;

			obj.push_back(wilson::consume_value(str));

			string_ops::trim_whitespace_left(str);

			if (!str.empty() && (str[0] == ',' || str[0] == ';'))
				str.remove_prefix(1);

		} while (!str.empty());
		return obj;
	}

	/// Consumes a string literal returning a string value
	inline std::string consume_string_literal(std::string_view& _str)
	{
		std::pair<std::string_view, std::string> result;
		if (_str[0] == '\'')
			result = parsing::consume_c_string<'\''>(_str);
		else
			result = parsing::consume_c_string<'"'>(_str);
		return std::move(result).second;
	}

	/// Consumes a word or a string literal returning a string value
	inline std::string consume_string_value(std::string_view& str)
	{
		string_ops::trim_whitespace_left(str);
		const auto first = str[0];
		if (first == '\'' || first == '"')
			return consume_string_literal(str);
		else if (string_ops::ascii::isidentstart(first))
			return std::string{ string_ops::consume_while(str, &string_ops::ascii::isident) };

		return {};
	}

	/// Consumes: a word, returning a string/bool/null value; or a "string literal", returning a string value
	inline nlohmann::json consume_word_or_string(std::string_view& str)
	{
		if (string_ops::ascii::isalpha(str[0]))
		{
			auto string = consume_string_value(str);
			if (string == "true")
				return true;
			else if (string == "false")
				return false;
			else if (string == "null" || string == "nil")
				return nullptr;
			return string;
		}
		else
		{
			return consume_string_value(str);
		}
	}

	inline nlohmann::json consume_object(std::string_view& str, char closing_char)
	{
		auto obj = nlohmann::json::object();

		do
		{
			string_ops::trim_whitespace_left(str);
			if (string_ops::consume(str, closing_char))
				return obj;

			auto key = consume_string_value(str);
			string_ops::trim_whitespace_left(str);

			if (str.empty() || string_ops::consume(str, closing_char))
			{
				obj[std::move(key)] = true;
				break;
			}

			if (str[0] == ',' || str[0] == ';')
			{
				obj[std::move(key)] = true;
			}
			else
			{
				if (str[0] == '=' || str[0] == ':')
					str.remove_prefix(1);

				string_ops::trim_whitespace_left(str);

				auto val = wilson::consume_value(str);
				obj[std::move(key)] = std::move(val);
			}

			string_ops::trim_whitespace_left(str);

			if (!str.empty() && (str[0] == ',' || str[0] == ';'))
				str.remove_prefix(1);
		} while (!str.empty());

		return obj;
	}

	inline nlohmann::json consume_value(std::string_view& str)
	{
		string_ops::trim_whitespace_left(str);
		if (str.empty()) return {};

		const auto first = str[0];
		if (string_ops::consume(str, '{'))
			return consume_object(str);
		else if (string_ops::consume(str, '('))
			return consume_array(str, ')');
		else if (string_ops::consume(str, '['))
			return consume_array(str, ']');
		else if (string_ops::ascii::isidentstart(first) || first == '\'' || first == '"')
			return consume_word_or_string(str);
		else if (string_ops::ascii::isdigit(first) || first == '-')
		{
			double result = 0;
			const auto fcresult = string_ops::from_chars(str, result);
			str = string_ops::make_sv(fcresult.ptr, str.end());
			return result;
		}
		return std::string(1, string_ops::consume(str)); /// eat at least 1 character so we don't fall into an infinite loop
	}

	inline auto parse(std::string_view wilson_str) -> nlohmann::json
	{
		const auto start = wilson_str.data(); /// TODO: on exception, get current wilson_str.data() and figure out line position, etc. to throw a proper exception
		return wilson::consume_value(wilson_str);
	}

	inline auto parse_object(std::string_view wilson_str, char closing_char) -> nlohmann::json
	{
		return wilson::consume_object(wilson_str, closing_char);
	}

	inline auto parse_array(std::string_view wilson_str) -> nlohmann::json
	{
		return wilson::consume_array(wilson_str);
	}

	inline auto parse_word_or_string(std::string_view wilson_str) -> nlohmann::json
	{
		return wilson::consume_word_or_string(wilson_str);
	}

	inline auto parse_string_value(std::string_view wilson_str) -> std::string
	{
		return wilson::consume_string_value(wilson_str);
	}

	inline auto parse_string_literal(std::string_view wilson_str) -> std::string
	{
		return wilson::consume_string_literal(wilson_str);
	}

	namespace detail
	{
		template <typename OUTFUNC, typename VAL>
		void output_value(OUTFUNC&& func, VAL const& val)
		{
			char temp_buffer[32]{};
			auto&& [end, ec] = std::to_chars(std::begin(temp_buffer), std::end(temp_buffer), val);
			func(string_ops::make_sv(std::begin(temp_buffer), end));
		}

		template <typename OUTFUNC>
		void output_string(OUTFUNC&& func, std::string_view strval)
		{
			func("\"");
			std::string buf;
			char temp[8]{};
			auto start = strval.begin();
			for (auto it = strval.begin(); it != strval.end(); ++it)
			{
				if (string_ops::ascii::isprint(*it) && *it != '"' && *it != '\\')
					continue;
				else
				{
					if (start != it)
						func(string_ops::make_sv(start, it));

					func("\\x");
					auto&& [end, ec] = std::to_chars(std::begin(temp), std::end(temp), static_cast<uint8_t>(*it), 16);
					func(string_ops::make_sv(std::begin(temp), end));

					start = std::next(it);
				}
			}
			if (start != strval.end())
				func(string_ops::make_sv(start, strval.end()));
			func("\"");
		}
	}

	template <typename OUTFUNC>
	void output(OUTFUNC&& out, nlohmann::json const& value)
	{
		switch (value.type())
		{
		using enum nlohmann::detail::value_t;
		case null: out("null"); return;
		case object:
			out("{ ");
			for (auto& [k, v] : value.get_ref<nlohmann::json::object_t const&>())
			{
				wilson::detail::output_string(out, k);
				out(": ");
				wilson::output(out, v);
				out(", ");
			}
			out(" }");
			return;
		case array:
			out("[ ");
			for (auto& obj : value.get_ref<nlohmann::json::array_t const&>())
			{
				wilson::output(out, obj);
				out(", ");
			}
			out(" ]");
			return;
		case string: wilson::detail::output_string(out, value.get_ref<nlohmann::json::string_t const&>()); return;
		case boolean: out(value.get_ref<nlohmann::json::boolean_t const&>() ? "true" : "false"); return;
		case number_integer: wilson::detail::output_value(out, value.get_ref<nlohmann::json::number_integer_t const&>()); return;
		case number_unsigned: wilson::detail::output_value(out, value.get_ref<nlohmann::json::number_unsigned_t const&>()); return;
		case number_float: wilson::detail::output_value(out, value.get_ref<nlohmann::json::number_float_t const&>()); return;
		case binary:
			out("[ ");
			for (auto byte : value.get_ref<nlohmann::json::binary_t const&>())
			{
				wilson::detail::output_value(out, byte);
				out(", ");
			}
			out(" ]");
			return;
		case discarded: return;
		}
	}

	inline std::string to_string(nlohmann::json const& value)
	{
		std::string result;
		wilson::output([&](std::string_view val) { result += val; }, value);
		return result;
	}

	inline nlohmann::json load_file(std::filesystem::path const& from, std::error_code& ec)
	{
		const auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? nlohmann::json{} : wilson::parse(std::string_view{ source });
	}

	inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json const& or_json)
	{
		std::error_code ec;
		const auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? or_json : wilson::parse(std::string_view{ source });
	}
}