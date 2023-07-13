/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include "string_ops.h"
#include "mmap.h"

namespace ghassanpl::formats
{
	
	namespace text
	{
		/// \defgroup Text Text
		/// \ingroup Formats
		/// @{
		
		/// Returns the contents of a text file as a string.
		/// \param ec is filled with the error if any happens
		inline std::string load_file(std::filesystem::path const& from, std::error_code& ec)
		{
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? std::string{} : std::string{ source.begin(), source.end() };
		}

		/// Returns the contents of a text file as a string.
		/// \exception std::runtime_error if file not found
		inline std::string load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec)
				throw std::runtime_error(format("file '{}' not found", from.string()));
			return std::string{ source.begin(), source.end() };
		}

		/// Returns the contents of a text file as a string, or an empty string on failure.
		inline std::string try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			return load_file(from, ec);
		}

		inline bool save_file(std::filesystem::path const& to, std::string_view string, std::error_code& ec)
		{
			/// TODO: How to fil ec?
			std::ofstream out{ to };
			out.write(string.data(), string.size());
			return out.fail();
		}

		inline void save_file(std::filesystem::path const& to, std::string_view string)
		{
			std::ofstream out{ to };
			out.exceptions(std::ios::badbit | std::ios::failbit);
			out.write(string.data(), string.size());
		}
		/// @}
	}

	namespace text_lines
	{

		/// \defgroup TextLines Text Lines
		/// \todo These are technically incorrect as they don't remove `\r` at split points
		/// \ingroup Formats
		/// @{
		
		inline std::vector<std::string> load_file(std::filesystem::path const& from, std::error_code& ec)
		{
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec)
				return {};

			std::vector<std::string> result;
			ghassanpl::string_ops::split(std::string_view{ source }, '\n', [&](std::string_view line, bool) { result.emplace_back(line); });
			return result;
		}

		inline std::vector<std::string> load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec)
				throw std::runtime_error(format("file '{}' not found", from.string()));

			std::vector<std::string> result;
			ghassanpl::string_ops::split(std::string_view{ source }, '\n', [&](std::string_view line, bool) { result.emplace_back(line); });
			return result;
		}

		template <typename CALLBACK>
		inline void load_file(std::filesystem::path const& from, CALLBACK&& callback)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec)
				throw std::runtime_error(format("file '{}' not found", from.string()));

			std::vector<std::string> result;
			ghassanpl::string_ops::split(std::string_view{ source }, '\n', [&](std::string_view line, bool) { callback(line); });
			return result;
		}

		inline std::vector<std::string> try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			return load_file(from, ec);
		}

		template <std::ranges::range T>
		inline bool save_file(std::filesystem::path const& to, T string_range, std::error_code& ec)
		{
			/// TODO: How to fil ec?
			std::ofstream out{ to };
			for (auto& string : string_range)
			{
				out.write(std::to_address(std::ranges::begin(string)), std::ranges::size(string));
				out << "\n";
			}
			return out.fail();
		}

		template <std::ranges::range T>
		inline void save_file(std::filesystem::path const& to, T string_range)
		{
			std::ofstream out{ to };
			out.exceptions(std::ios::badbit | std::ios::failbit);
			for (auto& string : string_range)
			{
				out.write(std::to_address(std::ranges::begin(string)), std::ranges::size(string));
				out << "\n";
			}
		}
		/// @}
	}

	namespace json
	{
		/// \defgroup JSON JSON
		/// \ingroup Formats
		/// @{

		inline const nlohmann::json empty_json = nlohmann::json{};
		inline const nlohmann::json empty_json_array = nlohmann::json::array();
		inline const nlohmann::json empty_json_object = nlohmann::json::object();

		inline nlohmann::json load_file(std::filesystem::path const& from, std::error_code& ec)
		{
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? nlohmann::json{} : nlohmann::json::parse(source);
		}

		inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json const& or_json = empty_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? or_json : nlohmann::json::parse(source);
		}

		inline nlohmann::json load_file(std::filesystem::path const& from)
		{
			try
			{
				return nlohmann::json::parse(ghassanpl::make_mmap_source<char>(from));
			}
			catch (...)
			{
				std::throw_with_nested(std::runtime_error{ format("while trying to load json file {}", from.string()) });
			}
		}

		inline void save_file(std::filesystem::path const& to, nlohmann::json const& j, bool pretty = true)
		{
			std::ofstream out{ to };
			nlohmann::detail::serializer<nlohmann::json> s{ nlohmann::detail::output_adapter<char, std::string>(out), '\t', nlohmann::detail::error_handler_t::strict };
			s.dump(j, pretty, false, 1);
		}

		/// Smaller name for \c nlohmann::json::value_t
		using jtype = nlohmann::json::value_t;

		/// Gets the item in the json object `g` with the key `key`, or an empty json object if none found.
		/// \param type the value must also be of this type
		inline nlohmann::json const& get(nlohmann::json const& g, std::string_view key, jtype type = jtype::discarded)
		{
			if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
				return *it;
			return json::empty_json;
		}
		
		/// Gets the value (converted to a string) in the json object `g` with the key `key`, or `default_value` if none found.
		/// \param type the value must also be of this type
		inline std::string get(nlohmann::json const& g, std::string_view key, std::string_view default_value, jtype type = jtype::discarded)
		{
			if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
				return (std::string)*it;
			return std::string{ default_value };
		}

		/// Gets the value (converted to an integer) in the json object `g` with the key `key`, or `default_value` if none found.
		/// \param type the value must also be of this type
		template <std::integral T>
		inline T get(nlohmann::json const& g, std::string_view key, T default_value, jtype type = jtype::discarded)
		{
			if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
				return (T)*it;
			return default_value;
		}

		/// Gets the value (converted to an floating point number) in the json object `g` with the key `key`, or `default_value` if none found.
		/// \param type the value must also be of this type
		template <std::floating_point T>
		inline T get(nlohmann::json const& g, std::string_view key, T default_value, jtype type = jtype::discarded)
		{
			if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
				return (T)*it;
			return default_value;
		}

		/// Gets the array value in the json object `g` with the key `key`, or an empty array if none found.
		inline nlohmann::json const& get_array(nlohmann::json const& g, std::string_view key)
		{
			if (auto it = g.find(key); it != g.end() && it->type() == jtype::array)
				return *it;
			return json::empty_json_array;
		}

		/// Sets the value of `val` to the item in json object `g` with key `key`
		/// \exception std::runtime_error on error (no key found, cannot convert json to `val` type, etc.)
		template <typename T>
		inline void field(T& val, nlohmann::json const& g, std::string_view key)
		{
			try
			{
				auto it = g.find(key);
				if (it != g.end())
				{
					val = *it;
					return;
				}
			}
			catch (...)
			{
				std::throw_with_nested(std::runtime_error{ std::format("while trying to convert value at key \"{}\" to type {}", key, typeid(T).name()) });
			}

			throw std::runtime_error(std::format("no key \"{}\" found", key));
		}

		/// Sets the value of `val` to the item in json array `g` at index `key`
		/// \exception std::runtime_error on error (invalid index, cannot convert json to `val` type, etc.)
		template <typename T>
		inline void field(T& val, nlohmann::json const& g, size_t key)
		{
			try
			{
				val = g.at(key);
				return;
			}
			catch (...)
			{
				std::throw_with_nested(std::runtime_error{ std::format("while trying to convert value at element {} to type {}", key, typeid(T).name()) });
			}
		}

		/// Same as \c field() but returns if it succeeded, instead of throwing.
		/// \see field()
		template <typename T>
		inline bool field_opt(T& val, nlohmann::json const& g, std::string_view key)
		{
			try
			{
				auto it = g.find(key);
				if (it != g.end())
				{
					val = *it;
					return true;
				}
			}
			catch (...)
			{
				std::throw_with_nested(std::runtime_error{ std::format("while trying to convert value at key \"{}\" to type {}", key, typeid(T).name()) });
			}
			return false;
		}

		/// Calls `func` with the actual value inside `j`; similar to `std::visit`
		template <typename VISIT_FUNC>
		void visit(nlohmann::json const& j, VISIT_FUNC&& func)
		{
			switch (j.type())
			{
			case nlohmann::json::value_t::object: func(j.get_ref<nlohmann::json::object_t const&>()); return;
			case nlohmann::json::value_t::array: func(j.get_ref<nlohmann::json::array_t const&>()); return;
			case nlohmann::json::value_t::string: func(j.get_ref<nlohmann::json::string_t const&>()); return;
			case nlohmann::json::value_t::boolean: func(j.get_ref<nlohmann::json::boolean_t const&>()); return;
			case nlohmann::json::value_t::number_integer: func(j.get_ref<nlohmann::json::number_integer_t const&>()); return;
			case nlohmann::json::value_t::number_unsigned: func(j.get_ref<nlohmann::json::number_unsigned_t const&>()); return;
			case nlohmann::json::value_t::number_float: func(j.get_ref<nlohmann::json::number_float_t const&>()); return;
			case nlohmann::json::value_t::binary: func(j.get_ref<nlohmann::json::binary_t const&>()); return;
			case nlohmann::json::value_t::null: func(nullptr); return;
			case nlohmann::json::value_t::discarded:
			default:
				break;
			}
		}

		/// @}
	}

	namespace ubjson
	{
		/// \defgroup UBJSON UBJSON
		/// \ingroup Formats
		/// @{
		inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json const& or_json = json::empty_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? or_json : nlohmann::json::from_ubjson(source);
		}

		inline nlohmann::json load_file(std::filesystem::path const& from, std::error_code& ec)
		{
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? nlohmann::json{} : nlohmann::json::from_ubjson(source);
		}

		inline nlohmann::json load_file(std::filesystem::path const& from)
		{
			try
			{
				return nlohmann::json::from_ubjson(ghassanpl::make_mmap_source<char>(from));
			}
			catch (...)
			{
				std::throw_with_nested(std::runtime_error{ format("while trying to load ubjson file {}", from.string()) });
			}
		}

		inline void save_file(std::filesystem::path const& to, nlohmann::json const& j)
		{
			std::ofstream out{ to };
			nlohmann::json::to_ubjson(j, nlohmann::detail::output_adapter<char, std::string>(out), true, true);
		}
		///@}
	}

	namespace cbor
	{
		/// \defgroup CBOR CBOR
		/// \ingroup Formats
		/// @{
		
		/// Tries loading a CBOR file
		inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json const& or_json = json::empty_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? or_json : nlohmann::json::from_cbor(source);
		}

		/// Saves a CBOR file
		inline void save_file(std::filesystem::path const& to, nlohmann::json const& j)
		{
			std::ofstream out{ to };
			nlohmann::json::to_cbor(j, nlohmann::detail::output_adapter<char, std::string>(out));
		}
		/// @}
	}

}