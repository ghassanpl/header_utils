#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include "mmap.h"

namespace ghassanpl
{
	inline std::string load_text_file(std::filesystem::path const& from, std::error_code& ec)
	{
		auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? std::string{} : std::string{ source.begin(), source.end() };
	}

	inline std::string load_text_file(std::filesystem::path const& from)
	{
		std::error_code ec;
		auto source = ghassanpl::make_mmap_source<char>(from, ec);
		if (ec)
			throw std::runtime_error(format("file '{}' not found", from.string()));
		return std::string{ source.begin(), source.end() };
	}

	inline bool save_text_file(std::filesystem::path const& to, std::string_view string, std::error_code& ec)
	{
		std::ofstream out{ to };
		out.write(string.data(), string.size());
		return out.fail();
	}

	inline void save_text_file(std::filesystem::path const& to, std::string_view string)
	{
		std::ofstream out{ to };
		out.exceptions(std::ios::badbit | std::ios::failbit);
		out.write(string.data(), string.size());
	}

	inline nlohmann::json load_json_file(std::filesystem::path const& from, std::error_code& ec)
	{
		auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? nlohmann::json{} : nlohmann::json::parse(source);
	}

	inline nlohmann::json load_ubjson_file(std::filesystem::path const& from, std::error_code& ec)
	{
		auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? nlohmann::json{} : nlohmann::json::from_ubjson(source);
	}

	static inline const nlohmann::json empty_json = nlohmann::json{};
	static inline const nlohmann::json empty_json_array = nlohmann::json::array();
	static inline const nlohmann::json empty_json_object = nlohmann::json::object();

	inline std::string try_load_text_file(std::filesystem::path const& from)
	{
		std::error_code ec;
		return load_text_file(from, ec);
	}

	inline nlohmann::json try_load_json_file(std::filesystem::path const& from, nlohmann::json const& or_json = empty_json)
	{
		std::error_code ec;
		auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? or_json : nlohmann::json::parse(source);
	}

	inline nlohmann::json try_load_ubjson_file(std::filesystem::path const& from, nlohmann::json const& or_json = empty_json)
	{
		std::error_code ec;
		auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? or_json : nlohmann::json::from_ubjson(source);
	}

	inline nlohmann::json try_load_cbor_file(std::filesystem::path const& from, nlohmann::json const& or_json = empty_json)
	{
		std::error_code ec;
		auto source = ghassanpl::make_mmap_source<char>(from, ec);
		return ec ? or_json : nlohmann::json::from_cbor(source);
	}

	inline nlohmann::json load_json_file(std::filesystem::path const& from)
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

	inline nlohmann::json load_ubjson_file(std::filesystem::path const& from)
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

	inline void save_json_file(std::filesystem::path const& to, nlohmann::json const& j)
	{
		std::ofstream out{ to };
		nlohmann::detail::serializer<nlohmann::json> s{ nlohmann::detail::output_adapter<char, std::string>(out), '\t', nlohmann::detail::error_handler_t::strict };
		s.dump(j, true, false, 1);
	}

	inline void save_ubjson_file(std::filesystem::path const& to, nlohmann::json const& j)
	{
		std::ofstream out{ to };
		nlohmann::json::to_ubjson(j, nlohmann::detail::output_adapter<char, std::string>(out), true, true);
	}

	inline void save_cbor_file(std::filesystem::path const& to, nlohmann::json const& j)
	{
		std::ofstream out{ to };
		nlohmann::json::to_cbor(j, nlohmann::detail::output_adapter<char, std::string>(out));
	}

	using jtype = nlohmann::json::value_t;

	inline nlohmann::json const& get(nlohmann::json const& g, std::string_view key, jtype type = jtype::discarded)
	{
		if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
			return *it;
		return empty_json;
	}

	inline std::string get(nlohmann::json const& g, std::string_view key, std::string_view default_value, jtype type = jtype::discarded)
	{
		if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
			return (std::string)*it;
		return std::string{ default_value };
	}

	template <std::integral T>
	inline T get(nlohmann::json const& g, std::string_view key, T default_value, jtype type = jtype::discarded)
	{
		if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
			return (T)*it;
		return default_value;
	}

	template <std::floating_point T>
	inline T get(nlohmann::json const& g, std::string_view key, T default_value, jtype type = jtype::discarded)
	{
		if (auto it = g.find(key); it != g.end() && (type == jtype::discarded || it->type() == type))
			return (T)*it;
		return default_value;
	}

	inline nlohmann::json const& get_array(nlohmann::json const& g, std::string_view key)
	{
		if (auto it = g.find(key); it != g.end() && it->type() == jtype::array)
			return *it;
		return empty_json_array;
	}

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
			std::throw_with_nested(std::runtime_error{ format("while trying to convert value at key \"{}\" to type {}", key, typeid(T).name()) });
		}

		throw std::runtime_error(format("no key \"{}\" found", key));
	}

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
			std::throw_with_nested(std::runtime_error{ format("while trying to convert value at element {} to type {}", key, typeid(T).name()) });
		}
	}

}