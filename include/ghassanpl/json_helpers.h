/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include "string_ops.h"
#include "mmap.h"
#include "expected.h"
#include "functional.h"

namespace ghassanpl::formats
{
	
	/*
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
			if (ec == std::errc::file_too_large) /// zero-sized file
				return {};
			if (ec)
				throw std::runtime_error(format("file '{}' could not be loaded: {}", from.string(), ec.message()));
			return std::string{ source.begin(), source.end() };
		}

		/// Returns the contents of a text file as a string
		inline expected<std::string, std::error_code> try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto result = load_file(from, ec);
			return ec ? unexpected(ec) : expected<std::string, std::error_code>{ std::move(result) };
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

			return resulting([&](std::vector<std::string>& result) {
				ghassanpl::string_ops::split({ source.begin(), source.end() }, '\n', op::emplace_back_to(result));
			});
		}

		inline std::vector<std::string> load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec == std::errc::file_too_large) /// zero-sized file
				return {};
			if (ec)
				throw std::runtime_error(format("file '{}' could not be loaded: {}", from.string(), ec.message()));

			return resulting([&](std::vector<std::string>& result) {
				ghassanpl::string_ops::split({ source.begin(), source.end() }, '\n', op::emplace_back_to(result));
			});
		}

		template <typename CALLBACK>
		requires std::invocable<CALLBACK, std::string_view>
		inline void load_file(std::filesystem::path const& from, CALLBACK&& callback)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec == std::errc::file_too_large) /// zero-sized file
				return;
			if (ec)
				throw std::runtime_error(format("file '{}' could not be loaded: {}", from.string(), ec.message()));

			ghassanpl::string_ops::split({ source.begin(), source.end() }, '\n', callback);
		}

		inline expected<std::vector<std::string>, std::error_code> try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto result = load_file(from, ec);
			return ec ? unexpected(ec) : expected<std::vector<std::string>, std::error_code>{ std::move(result) };
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
	*/

	namespace json
	{
		/// \defgroup JSON JSON
		/// \ingroup Formats
		/// @{

		inline const nlohmann::json empty_json = nlohmann::json{};
		inline const nlohmann::json empty_json_array = nlohmann::json::array();
		inline const nlohmann::json empty_json_object = nlohmann::json::object();

		/// Smaller name for \c nlohmann::json::value_t
		using jtype = nlohmann::json::value_t;

		inline expected<nlohmann::json, std::error_code> load_file(std::filesystem::path const& from)
		{
			std::error_code ec{};
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec != std::error_code{})
				return unexpected(ec);
			return nlohmann::json::parse(source);
		}

		inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json or_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? std::move(or_json) : nlohmann::json::parse(source);
		}

		inline nlohmann::json try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? empty_json : nlohmann::json::parse(source);
		}

		inline void save_file(std::filesystem::path const& to, nlohmann::json const& j, bool pretty = true)
		{
			std::ofstream out{ to };
			nlohmann::detail::serializer<nlohmann::json> s{ nlohmann::detail::output_adapter<char, std::string>(out), '\t', nlohmann::detail::error_handler_t::strict };
			s.dump(j, pretty, false, 1);
		}

		constexpr const char* type_name(nlohmann::json::value_t type) noexcept
		{
			switch (type)
			{
				using enum nlohmann::detail::value_t;
			case null:
				return "null";
			case object:
				return "object";
			case array:
				return "array";
			case string:
				return "string";
			case boolean:
				return "boolean";
			case binary:
				return "binary";
			case discarded:
				return "discarded";
			default:
				return "number";
			}
		}

		/// Calls `func` with the actual value inside `j`; similar to `std::visit`
		template <typename VISIT_FUNC>
		auto visit(nlohmann::json const& j, VISIT_FUNC&& func)
		{
			switch (j.type())
			{
				using enum nlohmann::detail::value_t;
			case object: return func(j.get_ref<nlohmann::json::object_t const&>());
			case array: return func(j.get_ref<nlohmann::json::array_t const&>());
			case string: return func(j.get_ref<nlohmann::json::string_t const&>());
			case boolean: return func(j.get_ref<nlohmann::json::boolean_t const&>());
			case number_integer: return func(j.get_ref<nlohmann::json::number_integer_t const&>());
			case number_unsigned: return func(j.get_ref<nlohmann::json::number_unsigned_t const&>());
			case number_float: return func(j.get_ref<nlohmann::json::number_float_t const&>());
			case binary: return func(j.get_ref<nlohmann::json::binary_t const&>());
			default:
				return func(nullptr);
			}
		}

		/// @}
	}

	namespace ubjson
	{
		/// \defgroup UBJSON UBJSON
		/// \ingroup Formats
		/// @{

		inline expected<nlohmann::json, std::error_code> load_file(std::filesystem::path const& from)
		{
			std::error_code ec{};
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec != std::error_code{})
				return unexpected(ec);
			return nlohmann::json::from_ubjson(source);
		}

		inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json or_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? std::move(or_json) : nlohmann::json::from_ubjson(source);
		}

		inline nlohmann::json try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? json::empty_json : nlohmann::json::from_ubjson(source);
		}

		inline expected<void, std::error_code> save_file(std::filesystem::path const& to, nlohmann::json const& j, bool pretty = true)
		{
			std::ofstream out;
			std::ios_base::iostate exceptionMask = out.exceptions() | std::ios::failbit;
			out.exceptions(exceptionMask);
			try
			{
				out.open(to, std::ios::binary);
			}
			catch (std::ios_base::failure& e)
			{
				return unexpected(e.code());
			}
			nlohmann::json::to_ubjson(j, nlohmann::detail::output_adapter<char, std::string>(out), true, true);
			return {};
		}

		/// @}
	}


	namespace cbor
	{
		/// \defgroup CBOR CBOR
		/// \ingroup Formats
		/// @{

		inline expected<nlohmann::json, std::error_code> load_file(std::filesystem::path const& from)
		{
			std::error_code ec{};
			auto source = ghassanpl::make_mmap_source<uint8_t>(from, ec);
			if (ec != std::error_code{})
				return unexpected(ec);
			return nlohmann::json::from_cbor(source);
		}

		inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json or_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<uint8_t>(from, ec);
			return ec ? std::move(or_json) : nlohmann::json::from_cbor(source);
		}

		inline nlohmann::json try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<uint8_t>(from, ec);
			return ec ? json::empty_json : nlohmann::json::from_cbor(source);
		}

		inline void save_file(std::filesystem::path const& to, nlohmann::json const& j, bool pretty = true)
		{
			std::ofstream out{ to, std::ios::binary };
			nlohmann::json::to_cbor(j, nlohmann::detail::output_adapter<char, std::string>(out));
		}

		/// @}
	}
	/*
	namespace json
	{
		/// \defgroup JSON JSON
		/// \ingroup Formats
		/// @{

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
	*/
}