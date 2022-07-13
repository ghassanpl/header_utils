#pragma once

#include "string_ops.h"
#include "mmap.h"
#include <nlohmann/json.hpp>
#include <ostream>

namespace ghassanpl
{
	nlohmann::json parse_wilson_value(std::string_view wilson_str);
	
	nlohmann::json parse_wilson_object(std::string_view wilson_str);
	nlohmann::json parse_wilson_array(std::string_view wilson_str);

	template <typename OUTFUNC>
	void output_wilson(OUTFUNC&& out, nlohmann::json const& value);

	inline void to_wilson_stream(std::ostream& strm, nlohmann::json const& value)
	{
		output_wilson([&](std::string_view val) { strm.write(val.data(), val.size()); }, value);
	}

	std::string to_wilson_string(nlohmann::json const& value);

	namespace detail
	{
		nlohmann::json parse_wilson(std::string_view& str);

		inline nlohmann::json parse_array(std::string_view& str)
		{
			auto obj = nlohmann::json::array();
			do
			{
				string_ops::trim_whitespace_left(str);
				if (string_ops::consume(str, ']') || string_ops::consume(str, ')'))
					return obj;

				obj.push_back(parse_wilson(str));

				string_ops::trim_whitespace_left(str);

				if (str[0] == ',' || str[0] == ';')
					str.remove_prefix(1);

			} while (!str.empty());
			return obj;
		}

		inline std::string parse_literal(std::string_view& _str)
		{
			std::pair<std::string_view, std::string> result;
			if (_str[0] == '\'')
				result = string_ops::consume_c_string<'\''>(_str);
			else
				result = string_ops::consume_c_string<'"'>(_str);
			return move(result).second;
		}

		inline std::string parse_identifier(std::string_view& from)
		{
			/*
			auto beg = from;
			std::ignore = string_ops::consume_while(from, &string_ops::ascii::isident);
			return string_ops::make_string(beg.begin(), from.begin());
			*/
			return string{ string_ops::consume_while(from, &string_ops::ascii::isident) };
		}

		inline std::string parse_string_value(std::string_view& str)
		{
			string_ops::trim_whitespace_left(str);
			auto first = str[0];
			if (first == '\'' || first == '"')
				return parse_literal(str);
			else if (string_ops::ascii::isalpha(first) || first == '_')
				return parse_identifier(str);

			return {};
		}

		inline nlohmann::json parse_string(std::string_view& str)
		{
			if (string_ops::ascii::isalpha(str[0]))
			{
				auto string = parse_string_value(str);
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
				return parse_string_value(str);
			}
		}

		inline nlohmann::json parse_object(std::string_view& str)
		{
			auto obj = nlohmann::json::object();

			do
			{
				string_ops::trim_whitespace_left(str);
				if (string_ops::consume(str, '}'))
					return obj;

				auto key = parse_string_value(str);
				string_ops::trim_whitespace_left(str);
				if (str[0] == ',' || str[0] == ';')
				{
					obj[std::move(key)] = true;
				}
				else
				{
					if (str[0] == '=' || str[0] == ':')
						str.remove_prefix(1);

					string_ops::trim_whitespace_left(str);

					auto val = parse_wilson(str);
					obj[std::move(key)] = std::move(val);
				}

				string_ops::trim_whitespace_left(str);

				if (str[0] == ',' || str[0] == ';')
					str.remove_prefix(1);
			} while (!str.empty());

			return obj;
		}

		inline nlohmann::json parse_wilson(std::string_view& str)
		{
			string_ops::trim_whitespace_left(str);
			if (str.empty()) return {};

			auto first = str[0];
			if (string_ops::consume(str, '{'))
				return parse_object(str);
			else if (string_ops::consume(str, '[') || string_ops::consume(str, '('))
				return parse_array(str);
			else if (string_ops::ascii::isalpha(first) || first == '\'' || first == '"')
				return parse_string(str);
			else if (string_ops::ascii::isdigit(first) || first == '-')
			{
				double result = 0;
				auto fcresult = std::from_chars(str.data(), str.data() + str.size(), result);
				str = string_ops::make_sv(fcresult.ptr, str.end());
				return result;
			}
			return {};
		}

		template <typename OUTFUNC, typename VAL>
		void output_value(OUTFUNC&& func, VAL&& val)
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

	inline nlohmann::json parse_wilson_value(std::string_view wilson_str)
	{
		return detail::parse_wilson(wilson_str);
	}

	inline nlohmann::json parse_wilson_object(std::string_view wilson_str)
	{
		return detail::parse_object(wilson_str);
	}

	inline nlohmann::json parse_wilson_array(std::string_view wilson_str)
	{
		return detail::parse_array(wilson_str);
	}

	template <typename OUTFUNC>
	void output_wilson(OUTFUNC&& out, nlohmann::json const& value)
	{
		using namespace nlohmann;
		switch (value.type())
		{
		case nlohmann::json::value_t::null: out("null"); return;
		case nlohmann::json::value_t::object:
			out("{ ");
			for (auto& obj : value.get_ref<json::object_t const&>())
			{
				detail::output_string(out, obj.first);
				out(": ");
				output_wilson(out, obj.second);
				out(", ");
			}
			out(" }");
			return;
		case nlohmann::json::value_t::array:
			out("[ ");
			for (auto& obj : value.get_ref<json::array_t const&>())
			{
				output_wilson(out, obj);
				out(", ");
			}
			out(" ]");
			return;
		case nlohmann::json::value_t::string: detail::output_string(out, value.get_ref<json::string_t const&>()); return;
		case nlohmann::json::value_t::boolean: out(value.get_ref<json::boolean_t const&>() ? "true" : "false"); return;
		case nlohmann::json::value_t::number_integer: detail::output_value(out, value.get_ref<json::number_integer_t const&>()); return;
		case nlohmann::json::value_t::number_unsigned: detail::output_value(out, value.get_ref<json::number_unsigned_t const&>()); return;
		case nlohmann::json::value_t::number_float: detail::output_value(out, value.get_ref<json::number_float_t const&>()); return;
		case nlohmann::json::value_t::binary:
			out("[ ");
			for (auto byte : value.get_ref<json::binary_t const&>())
			{
				detail::output_value(out, byte);
				out(", ");
			}
			out(" ]");
			return;
		case nlohmann::json::value_t::discarded: return;
		}
	}

	inline std::string to_wilson_string(nlohmann::json const& value)
	{
		std::string result;
		output_wilson([&](std::string_view val) { result += val; }, value);
		return result;
	}

	inline nlohmann::json load_wilson_file(std::filesystem::path const& from, std::error_code& ec)
	{
		auto source = ghassanpl::make_mmap_source(from, ec);
		return ec ? nlohmann::json{} : parse_wilson_value(string_ops::make_sv((const char*)source.begin(), (const char*)source.end()));
	}

	inline nlohmann::json try_load_wilson_file(std::filesystem::path const& from, nlohmann::json const& or_json = empty_json)
	{
		std::error_code ec;
		auto source = ghassanpl::make_mmap_source(from, ec);
		return ec ? or_json : parse_wilson_value(string_ops::make_sv((const char*)source.begin(), (const char*)source.end()));
	}

}