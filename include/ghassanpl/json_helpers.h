/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
//#include "formats.h"
#include "unicode.h"
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


		/// Gets the item in the json object `g` with the key `key`, or an empty json object if none found.
		/// \param type the value must also be of this type
		[[nodiscard]] inline nlohmann::json const& get(nlohmann::json const& g, std::string_view key, jtype type = jtype::discarded)
		{
			if (auto const it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
				return *it;
			return empty_json;
		}

		[[nodiscard]] inline nlohmann::json const* get_ptr(nlohmann::json const& g, std::string_view key, jtype type = jtype::discarded)
		{
			if (auto const it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
				return &*it;
			return nullptr;
		}

		/// Gets the array value in the json object `g` with the key `key`, or an empty array if none found.
		[[nodiscard]] inline nlohmann::json const& get_array(nlohmann::json const& g, std::string_view key)
		{
			if (auto const it = g.find(key); it != g.end() && it->type() == jtype::array)
				return *it;
			return empty_json_array;
		}

		/// Gets the object value in the json object `g` with the key `key`, or an empty array if none found.
		[[nodiscard]] inline nlohmann::json const& get_object(nlohmann::json const& g, std::string_view key)
		{
			if (auto const it = g.find(key); it != g.end() && it->type() == jtype::object)
				return *it;
			return empty_json_object;
		}

		/// Gets the value from the item in json object `g` with key `key`, to `val`
		/// \exception std::runtime_error on error (no key found, cannot convert json to `val` type, etc.)
		template <typename T>
		inline void get_field(T& val, nlohmann::json const& g, std::string_view key)
		{
			auto const it = g.find(key);
			if (it == g.end())
				throw std::runtime_error(std::format("no key \"{}\" found", key));
			try
			{
				val = *it;
			}
			catch (...)
			{
				std::throw_with_nested(std::runtime_error{ std::format("while trying to convert value at key \"{}\" to type {}", key, typeid(T).name()) });
			}

		}

		/// Same as \c field() but returns false if it fails, instead of throwing.
		/// \returns false if key is not found, or cannot be converted
		/// \see field()
		template <typename T>
		inline bool get_field_opt(T& val, nlohmann::json const& g, std::string_view key)
		{
			auto const it = g.find(key);
			if (it == g.end())
				return false;

			try
			{
				val = *it;
				return true;
			}
			catch (...)
			{
				//std::throw_with_nested(std::runtime_error{ std::format("while trying to convert value at key \"{}\" to type {}", key, typeid(T).name()) });
			}
		}

		/// \exception std::runtime_error on error (no key found, cannot convert json to `val` type, etc.)
		template <typename T>
		[[nodiscard]] T get_field_val(nlohmann::json const& g, std::string_view key)
		{
			try
			{
				auto const it = g.find(key);
				if (it != g.end())
					return *it;
			}
			catch (...)
			{
				std::throw_with_nested(std::runtime_error{ std::format("while trying to convert value at key \"{}\" to type {}", key, typeid(T).name()) });
			}

			throw std::runtime_error(std::format("no key \"{}\" found", key));
		}

		template <typename T>
		[[nodiscard]] T get_field_val_or_default(nlohmann::json const& g, std::string_view key, T&& default_val = T{})
		{
			try
			{
				auto const it = g.find(key);
				if (it != g.end())
					return *it;
			}
			catch (...)
			{
			}
			return std::forward<T>(default_val);
		}

		/// @}
	}

	namespace ubjson
	{
		/// \defgroup UBJSON UBJSON
		/// \ingroup Formats
		/// @{

		[[nodiscard]] inline expected<nlohmann::json, std::error_code> load_file(std::filesystem::path const& from)
		{
			std::error_code ec{};
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			if (ec != std::error_code{})
				return unexpected(ec);
			return nlohmann::json::from_ubjson(source);
		}

		[[nodiscard]] inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json or_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? std::move(or_json) : nlohmann::json::from_ubjson(source);
		}

		[[nodiscard]] inline nlohmann::json try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<char>(from, ec);
			return ec ? json::empty_json : nlohmann::json::from_ubjson(source);
		}

		[[nodiscard]] inline expected<void, std::error_code> save_file(std::filesystem::path const& to, nlohmann::json const& j)
		{
			std::ofstream out;
			out.exceptions(out.exceptions() | std::ios::failbit);
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

		[[nodiscard]] inline expected<nlohmann::json, std::error_code> load_file(std::filesystem::path const& from)
		{
			std::error_code ec{};
			auto source = ghassanpl::make_mmap_source<uint8_t>(from, ec);
			if (ec != std::error_code{})
				return unexpected(ec);
			return nlohmann::json::from_cbor(source);
		}

		[[nodiscard]] inline nlohmann::json try_load_file(std::filesystem::path const& from, nlohmann::json or_json)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<uint8_t>(from, ec);
			return ec ? std::move(or_json) : nlohmann::json::from_cbor(source);
		}

		[[nodiscard]] inline nlohmann::json try_load_file(std::filesystem::path const& from)
		{
			std::error_code ec;
			auto source = ghassanpl::make_mmap_source<uint8_t>(from, ec);
			return ec ? json::empty_json : nlohmann::json::from_cbor(source);
		}

		inline void save_file(std::filesystem::path const& to, nlohmann::json const& j)
		{
			std::ofstream out{ to, std::ios::binary };
			nlohmann::json::to_cbor(j, nlohmann::detail::output_adapter<char, std::string>(out));
		}

		/// @}
	}

	/// TODO: UNTESTED
	namespace csv
	{
		namespace detail
		{
			inline std::string_view ensure_delimited_for_csv(std::string_view str, std::string& temp, bool raw)
			{
				if (raw)
					return str;

				std::string delimited;
				std::string* out = nullptr;
				for (size_t i = 0; i < str.size(); ++i)
				{
					const char c = str[i];
					if (c == '\r' || c == '\n' || c == '"' || c == ',')
					{
						if (!out)
						{
							out = &delimited;
							*out += '"';
							*out += str.substr(0, i);
						}

						if (c == '"')
							*out += '"';
						*out += c;
					}
					else
					{
						if (out)
						{
							delimited += c;
						}
						else
						{
							/// No delimiters so far, we can keep returning the original string view
						}
					}
				}

				if (out)
				{
					*out += '"';
					temp = std::move(delimited);
					return temp;
				}

				/// No delimiters, we can return the original string view without copying
				return str;
			}

			/// Takes a JSON value or a stringable and returns its (potentially delimited) string representation as a string_view;
			/// tries to not allocate new string data if not necessary, but if it does, it does so in the `temp` object
			template <typename T>
			std::string_view to_csv_string(T const& val, std::string& temp, bool raw)
			{
				if constexpr (std::same_as<std::remove_cvref_t<T>, nlohmann::json>)
				{
					if (val.is_string())
						return ensure_delimited_for_csv(val.template get_ref<nlohmann::json::string_t const&>(), temp, raw);
					else if (val.is_primitive()) /// Bools, nulls and numbers will never be delimited, but still need to be strringified
					{
						temp = to_string(val);
						return temp;
					}
					else
					{
						/// Worst case, we convert the JSON to a string, then delimit it, 
						/// potentially allocating twice
						/// NOTE: TODO: Technically, we could reuse `temp` storage and insert the delimiters in there,
						/// avoiding allocating twice, but this is a bit too premature for now
						temp = to_string(val);
						return ensure_delimited_for_csv(temp, temp, raw);
					}
				}
				else /// Else something stringable
				{
					return ensure_delimited_for_csv(val, temp, raw);
				}
			}

			template <typename OUTPUT_TYPE>
			auto outputter_for(OUTPUT_TYPE&& output)
			{
				if constexpr (std::same_as<OUTPUT_TYPE, std::string&>) /// TODO: Technically could be any string specialization, and we could `transcode_unicode`
				{
					return outputter_for([&](std::string_view str) {
						output += str;
					});
				}
				else if constexpr (std::same_as<std::filesystem::path, std::remove_cvref_t<OUTPUT_TYPE>>)
				{
					std::ofstream file{ output, std::ios::binary };
					return outputter_for([file = std::move(file)](std::string_view value) mutable {
						file.write(value.data(), std::streamsize(value.size()));
					});
				}
				else if constexpr (std::is_lvalue_reference_v<OUTPUT_TYPE> && std::is_base_of_v<std::ostream, std::remove_cvref_t<OUTPUT_TYPE>>)
				{
					return outputter_for([&](std::string_view value) {
						output.write(value.data(), std::streamsize(value.size()));
					});
				}
				else if constexpr (std::invocable<OUTPUT_TYPE, std::string_view>)
				{
					/// Hack to capture by forwarding reference
					struct { OUTPUT_TYPE output; } cap{ std::forward<OUTPUT_TYPE>(output) };
					return [cap = std::move(cap)](auto&& value, bool raw) mutable {
						std::string temp;
						cap.output(to_csv_string(std::forward<decltype(value)>(value), temp, raw));
					};
				}
				else
				{
					static_assert(!std::same_as<std::void_t<OUTPUT_TYPE>, void>, "Cannot use this output");
				}
			}
		}

		/// Converts a JSON array of objects into CSV, assigning each object value to a specified column;
		/// does the minimum amount of allocations reasonable
		/// TODO: UNTESTED
		template <typename OUTPUT_TYPE>
		void json_to_csv(nlohmann::json const& j, OUTPUT_TYPE& output, std::span<const std::string_view> column_names)
		{
			if (!j.is_array())
				throw std::invalid_argument("json must be an array of objects");

			const nlohmann::json empty_string = "";

			auto outputter = detail::outputter_for(output);

			/// Columns
			bool first = false;
			for (auto& column_name : column_names)
			{
				if (std::exchange(first, true)) outputter(",", true);
				outputter(column_name, false);
			}
			outputter("\r\n", true);

			/// Rows
			std::vector<nlohmann::json const*> row;
			for (auto& item : j)
			{
				if (!item.is_object())
					throw std::invalid_argument("json must be an array of objects");

				row.clear();
				row.resize(column_names.size(), &empty_string);

				for (auto& [key, value] : item.items())
				{
					if (auto it = std::ranges::find(column_names, key); it != column_names.end())
					{
						const auto column_index = (it - column_names.begin());

						/// NOTE: TODO: Technically we COULD rely on the fact that the keys are ordered, and not search from the beginning every time...
						/// e.g. 
						///		auto columns_skipped = (it - column_names.begin());
						///		auto column_index = columns_skipped + prev_column_index;
						///		column_names_copy = column_names_copy.subspan(column_index+1);
						///		while (columns_skipped--) outputter(","); /// Skipped cells
						///		outputter(value);
						///		outputter(",");
						///		prev_column_index = column_index;
						/// 
						/// This would allow us to not allocate the `row` vector

						row[column_index] = &value;
					}
				}

				first = false;
				for (auto& cell : row)
				{
					if (std::exchange(first, true)) outputter(",", true);
					outputter(*cell, false);
				}
				outputter("\r\n", true);
			}
		}


		/// TODO: Very nice and very cool but needs C++23 which means we should probably move it to a different header

#if 0
		/// Converts a JSON array of objects into CSV, guessing column names;
		/// does the minimum amount of allocations reasonable
		/// TODO: UNTESTED
		template <typename OUTPUT_TYPE>
		void json_to_csv(nlohmann::json const& j, OUTPUT_TYPE&& output)
		{
			if (!j.is_array())
				throw std::invalid_argument("json must be an array of objects");
			
			/// TODO: Technically, we could walk through the array twice, once gathering column names,
			/// then dispatching to `json_to_csv` that takes the column names... 

			const nlohmann::json empty_string = "";

			const size_t row_count = j.size();
			std::map<std::string_view, std::vector<nlohmann::json const*>> columns;

			size_t row_num = 0;
			for (auto& item : j)
			{
				if (!item.is_object())
					throw std::invalid_argument("json must be an array of objects");

				for (auto& [key, value] : item.items())
				{
					if (!columns.contains(key))
						columns[key].resize(row_count, &empty_string);
					columns[key][row_num] = &value;
				}

				++row_num;
			}

			std::vector<std::string_view> column_names;
			column_names.assign_range(std::views::keys(columns));

			auto outputter = detail::outputter_for(output);

			bool first = false;
			for (auto& column_name : column_names)
			{
				if (std::exchange(first, true)) outputter(",", true);
				outputter(column_name, false);
			}
			outputter("\r\n", true);

			for (size_t i = 0; i < row_count; i++)
			{
				bool first = false;
				for (auto& column_name : column_names)
				{
					if (std::exchange(first, true)) outputter(",", true);
					outputter(*columns[column_name][i], false);
				}
				outputter("\r\n", true);
			}
		}

		template <typename OUTPUT_TYPE>
		void json_to_csv(nlohmann::json const& j, OUTPUT_TYPE&& output, std::string_view column_name, std::convertible_to<std::string_view> auto... column_names)
		{
			json_to_csv(j, std::forward<OUTPUT_TYPE>(output), std::array{ column_name, std::string_view{ column_names }... });
		}

		/// Usage:
		/// struct Entry
		/// {
		/// 	ItemCategory item_category;
		/// 	int	item_type;
		/// 	int	price;
		/// 	int	count;
		/// };
		/// 
		/// csv_outputter outputter{ filename, column_names };
		/// 
		/// for (Entry const & entry : entries)
		/// {
		/// 	outputter(entry.item_category);
		/// 	outputter(entry.item_type);
		/// 	outputter(entry.price);
		/// 	outputter(entry.count);
		/// }

		struct csv_outputter
		{
			template <typename OUTPUTTER>
			csv_outputter(OUTPUTTER&& outputter, std::span<const std::string_view> column_names)
				: m_column_names(column_names)
			{
				if (column_names.empty())
					throw std::invalid_argument("column names must not be empty");

				m_outputter = [outputter = detail::outputter_for(std::forward<OUTPUTTER>(outputter))](std::string_view cell, bool raw) mutable {
					outputter(cell, raw);
				};

				for (auto& column : column_names)
					output_next_cell(column);
			}

			template <typename OUTPUTTER>
			csv_outputter(OUTPUTTER&& outputter, std::convertible_to<std::string_view> auto... columns)
				: csv_outputter(outputter, std::initializer_list{ std::string_view{columns}... })
			{

			}

			/// TODO: Support non-linear outputting via operator()(size_t/string_view column_id, T&& val)

			template <typename T>
			csv_outputter& operator()(T&& val)
			{
				using std::to_string;
				std::string temp;
				std::string_view sv;

				if constexpr (std::constructible_from<std::string_view, T&&>)
				{
					sv = std::string_view{ val };
				}
				else
				{
					temp = to_string(std::forward<T>(val));
					sv = temp;
				}

				sv = ghassanpl::formats::csv::detail::ensure_delimited_for_csv(sv, temp, false);

				output_next_cell(sv);

				return *this;
			}

			void output_next_cell(std::string_view sv)
			{
				if (m_current_column != 0)
					m_outputter(",", true);
				m_outputter(sv, false);
				if (++m_current_column == m_column_names.size())
				{
					m_current_column = 0;
					m_outputter("\r\n", true);
				}
			}

			void next_row()
			{
				do
				{
					this->operator()(std::string_view{});
				} while (m_current_column != 0);
			}

			auto const& column_names() const { return m_column_names; }
			size_t current_column() const { return m_current_column; }
			auto const& current_column_name() const { return m_column_names[m_current_column]; }

		private:

			std::span<const std::string_view> m_column_names;
			size_t m_current_column = 0;

			std::move_only_function<void(std::string_view, bool)> m_outputter;
		};

#endif
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