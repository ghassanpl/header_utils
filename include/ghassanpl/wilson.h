/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "parsing.h"
#include "mmap.h"
#include "json_helpers.h"
#include "expected.h"
#include <ostream>

/// TODO: https://dev.stenway.com/SML/Examples.html

namespace ghassanpl::formats::wilson
{
	struct wilson_parsing_error { const char* where; std::string message; };

	using wilson = nlohmann::json;

	auto parse(std::string_view wilson_str) -> expected<wilson, wilson_parsing_error>;
	auto parse_object(std::string_view wilson_str, char closing_char = '}') -> expected<wilson, wilson_parsing_error>;
	auto parse_array(std::string_view wilson_str) -> expected<wilson, wilson_parsing_error>;
	/// Parses: a word as a string value or bool/null value; a "string literal" as a string value
	auto parse_word_or_string(std::string_view wilson_str) -> expected<wilson, wilson_parsing_error>;
	/// Parses a word or a string literal a string value
	auto parse_string_value(std::string_view wilson_str) -> expected<std::string, wilson_parsing_error>;
	/// Parses a string literal as a string value
	auto parse_string_literal(std::string_view wilson_str) -> expected<std::string, wilson_parsing_error>;

	auto consume_string_literal(std::string_view& _str) -> expected<std::string, wilson_parsing_error>;
	auto consume_string_value(std::string_view& str) -> expected<std::string, wilson_parsing_error>;
	auto consume_word_or_string(std::string_view& str) -> expected<wilson, wilson_parsing_error>;
	auto consume_object(std::string_view& wilson_str, char closing_char = '}') -> expected<wilson, wilson_parsing_error>;
	auto consume_array(std::string_view& wilson_str, char closing_char = ']') -> expected<wilson, wilson_parsing_error>;
	auto consume_value(std::string_view& wilson_str) -> expected<wilson, wilson_parsing_error>;

	struct output_parameters
	{
		bool pretty = false;
		/// bool output_commas = true;
		std::string_view indent_str = "\t";
	};

	template <typename OUTFUNC>
	void output(OUTFUNC&& out, wilson const& value, output_parameters const& parameters = {}, size_t indent = 0);

	inline void output_to_stream(std::ostream& strm, wilson const& value, output_parameters const& parameters = {})
	{
		output([&](std::string_view val) { strm.write(val.data(), val.size()); }, value, parameters);
	}

	std::string to_string(wilson const& value, output_parameters const& parameters = {});

	expected<wilson, wilson_parsing_error> load_file(std::filesystem::path const& from);
	wilson try_load_file(std::filesystem::path const& from, wilson or_json = json::empty_json);

	/// TODO: Add ec version
	inline void save_file(std::filesystem::path const& to, wilson const& j, output_parameters const& parameters = {})
	{
		std::ofstream out{ to };
		if (!out)
			throw std::runtime_error{ "could not open file for writing" };
		formats::wilson::output_to_stream(out, j, parameters);
	}
}

namespace ghassanpl::formats::wilson
{
	inline expected<wilson, wilson_parsing_error> consume_array(std::string_view& str, char closing_char)
	{
		auto obj = wilson::array();
		do
		{
			string_ops::trim_whitespace_left(str);
			if (string_ops::consume(str, closing_char))
				return obj;

			auto element = formats::wilson::consume_value(str);
			if (!element)
				return element;
			obj.push_back(std::move(element.value()));

			string_ops::trim_whitespace_left(str);

			if (!str.empty() && (str[0] == ',' || str[0] == ';'))
				str.remove_prefix(1);

		} while (!str.empty());
		return obj;
	}

	/// Consumes a string literal returning a string value
	inline expected<std::string, wilson_parsing_error> consume_string_literal(std::string_view& _str)
	{
		std::pair<std::string_view, std::string> result;
		if (_str[0] == '\'')
			result = parsing::consume_c_string<'\''>(_str);
		else
			result = parsing::consume_c_string<'"'>(_str);
		return std::move(result).second;
	}

	/// Consumes a word or a string literal returning a string value
	inline expected<std::string, wilson_parsing_error> consume_string_value(std::string_view& str)
	{
		string_ops::trim_whitespace_left(str);
		if (!str.empty())
		{
			const auto first = str[0];
			if (first == '\'' || first == '"')
				return consume_string_literal(str);
			else if (string_ops::ascii::isidentstart(first))
				return std::string{ string_ops::consume_while(str, &string_ops::ascii::isident) };
		}

		return unexpected(wilson_parsing_error{ str.data(), "expected quote character or identifier" });
	}

	/// Consumes: a word, returning a string/bool/null value; or a "string literal", returning a string value
	inline expected<wilson, wilson_parsing_error> consume_word_or_string(std::string_view& str)
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
		return consume_string_value(str);
	}

	inline expected<wilson, wilson_parsing_error> consume_object(std::string_view& str, char closing_char)
	{
		auto obj = wilson::object();

		do
		{
			string_ops::trim_whitespace_left(str);
			if (string_ops::consume(str, closing_char))
				return obj;

			auto key = consume_string_value(str);
			if (!key)
				return key;

			string_ops::trim_whitespace_left(str);

			if (str.empty() || string_ops::consume(str, closing_char))
			{
				obj[std::move(key.value())] = true;
				break;
			}

			if (str[0] == ',' || str[0] == ';')
			{
				obj[std::move(key.value())] = true;
			}
			else
			{
				if (str[0] == '=' || str[0] == ':')
					str.remove_prefix(1);

				string_ops::trim_whitespace_left(str);

				auto val = formats::wilson::consume_value(str);
				if (!val)
					return val;
				obj[std::move(key.value())] = std::move(val.value());
			}

			string_ops::trim_whitespace_left(str);

			if (!str.empty() && (str[0] == ',' || str[0] == ';'))
				str.remove_prefix(1);
		} while (!str.empty());

		return obj;
	}

	inline expected<wilson, wilson_parsing_error> consume_value(std::string_view& str)
	{
		string_ops::trim_whitespace_left(str);
		if (str.empty()) return unexpected(wilson_parsing_error{str.data(), "expected value"});

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
			if (fcresult.ec == std::errc{})
			{
				str = string_ops::make_sv(fcresult.ptr, str.end());
				return result;
			}
		}

		return unexpected(wilson_parsing_error{ str.data(), "expected object, array, or valid scalar" });
	}

	inline auto parse(std::string_view wilson_str) -> expected<wilson, wilson_parsing_error>
	{
		const auto start = wilson_str.data(); /// TODO: on exception, get current wilson_str.data() and figure out line position, etc. to throw a proper exception
		return formats::wilson::consume_value(wilson_str);
	}

	inline auto parse_object(std::string_view wilson_str, char closing_char) -> expected<wilson, wilson_parsing_error>
	{
		return formats::wilson::consume_object(wilson_str, closing_char);
	}

	inline auto parse_array(std::string_view wilson_str) -> expected<wilson, wilson_parsing_error>
	{
		return formats::wilson::consume_array(wilson_str);
	}

	inline auto parse_word_or_string(std::string_view wilson_str) -> expected<wilson, wilson_parsing_error>
	{
		return formats::wilson::consume_word_or_string(wilson_str);
	}

	inline auto parse_string_value(std::string_view wilson_str) -> expected<std::string, wilson_parsing_error>
	{
		return formats::wilson::consume_string_value(wilson_str);
	}

	inline auto parse_string_literal(std::string_view wilson_str) -> expected<std::string, wilson_parsing_error>
	{
		return formats::wilson::consume_string_literal(wilson_str);
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

					/// NOTE: This is just for reasonable printing, we could use \x for everything
					/// TODO: Maybe make this optional?
					switch (*it)
					{
					case 'n': func("\\n"); goto reset_start_to_next_char;
					case '"': func("\\\""); goto reset_start_to_next_char;
					case '\'': func("\\'"); goto reset_start_to_next_char;
					case '\\': func("\\\\"); goto reset_start_to_next_char;
					case 'b': func("\\b"); goto reset_start_to_next_char;
					case 'r': func("\\r"); goto reset_start_to_next_char;
					case 'f': func("\\f"); goto reset_start_to_next_char;
					case 't': func("\\t"); goto reset_start_to_next_char;
					case '0': func("\\0"); goto reset_start_to_next_char;
					default: break;
					}

					{
						func("\\x");
						auto&& [end, ec] = std::to_chars(std::begin(temp), std::end(temp), static_cast<uint8_t>(*it), 16);
						func(string_ops::make_sv(std::begin(temp), end));
					}

				reset_start_to_next_char:
					start = std::next(it);
				}
			}
			if (start != strval.end())
				func(string_ops::make_sv(start, strval.end()));
			func("\"");
		}
	}

	namespace detail
	{
		template <typename OUTFUNC>
		void do_indent(OUTFUNC&& out, output_parameters const& parameters, size_t indent)
		{
			for (size_t i=0; i<indent; ++i)
				out(parameters.indent_str);
		}
	}

	template <typename OUTFUNC>
	void output(OUTFUNC&& out, wilson const& value, output_parameters const& parameters, size_t indent)
	{
		const auto pretty = parameters.pretty;
		switch (value.type())
		{
		using enum nlohmann::detail::value_t;
		case null: out("null"); return;
		case object:
			out(pretty ? "{\n" : "{ ");
			for (auto& [k, v] : value.get_ref<wilson::object_t const&>())
			{
				if (pretty)
					detail::do_indent(out, parameters, indent + 1);
				formats::wilson::detail::output_string(out, k);
				out(": ");
				formats::wilson::output(out, v, parameters, indent + 1);
				out(pretty ? ",\n" : ", ");
			}
			if (pretty)
				detail::do_indent(out, parameters, indent);
			out("}");
			return;
		case array:
			out(pretty ? "[\n" : "[ ");
			for (auto& obj : value.get_ref<wilson::array_t const&>())
			{
				if (pretty)
					detail::do_indent(out, parameters, indent + 1);
				formats::wilson::output(out, obj, parameters, indent + 1);
				out(pretty ? ",\n" : ", ");
			}
			if (pretty)
				detail::do_indent(out, parameters, indent);
			out("]");
			return;
		case string: formats::wilson::detail::output_string(out, value.get_ref<wilson::string_t const&>()); return;
		case boolean: out(value.get_ref<wilson::boolean_t const&>() ? "true" : "false"); return;
		case number_integer: formats::wilson::detail::output_value(out, value.get_ref<wilson::number_integer_t const&>()); return;
		case number_unsigned: formats::wilson::detail::output_value(out, value.get_ref<wilson::number_unsigned_t const&>()); return;
		case number_float: formats::wilson::detail::output_value(out, value.get_ref<wilson::number_float_t const&>()); return;
		case binary: {
			out(pretty ? "[\n" : "[ ");
			size_t count = 0;
			for (auto byte : value.get_ref<wilson::binary_t const&>())
			{
				if (pretty && (count % 16) == 0)
					detail::do_indent(out, parameters, indent + 1);
				formats::wilson::detail::output_value(out, byte);
				++count;
				out((pretty && (count % 16) == 0) ? ",\n" : ", ");
			}
			if (pretty)
			{
				out("\n");
				detail::do_indent(out, parameters, indent);
			}
			out("]");
			return;
		}
		case discarded: return;
		}
	}

	inline std::string to_string(wilson const& value, output_parameters const& parameters)
	{
		std::string result;
		formats::wilson::output(op::append_to(result), value, parameters);
		return result;
	}

	inline expected<wilson, wilson_parsing_error> load_file(std::filesystem::path const& from)
	{
		std::error_code ec;
		const auto source = ghassanpl::make_mmap_source<char>(from, ec);
		if (ec)
			return unexpected(wilson_parsing_error{ {}, ec.message() });
		return formats::wilson::parse(std::string_view{source.begin(), source.end()});
	}

	inline wilson try_load_file(std::filesystem::path const& from, wilson or_json)
	{
		return formats::wilson::load_file(from).value_or(std::move(or_json));
	}

}