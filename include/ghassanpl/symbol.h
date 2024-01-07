/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <string_view>
#include <set>

namespace ghassanpl
{
	template <typename SYMBOL_PROVIDER>
	struct symbol_base
	{
		using symbol_provider = SYMBOL_PROVIDER;
		using internal_value_type = typename symbol_provider::internal_value_type;
		using hash_type = typename symbol_provider::hash_type;

		internal_value_type value = symbol_provider::empty_value();

		explicit symbol_base(std::string_view val) : value{ symbol_provider::insert(val) } { }
		symbol_base() noexcept = default;

		hash_type get_hash() const noexcept { return symbol_provider::hash_for(value); }
		std::string_view get_string() const noexcept { return symbol_provider::string_for(value); }
		explicit operator std::string_view() const noexcept { return symbol_provider::string_for(value); }

		auto operator->() const noexcept requires std::is_pointer_v<internal_value_type> { return value; }

		bool operator==(symbol_base const& other) const noexcept { return value == other.value; }
		auto operator<=>(symbol_base const& other) const noexcept { return value <=> other.value; }

		friend bool operator==(std::string_view a, symbol_base const& b) noexcept { return a == b.get_string(); }
		friend auto operator<=>(std::string_view a, symbol_base const& b) noexcept { return a <=> b.get_string(); }
	};
}

template <typename SYMBOL_PROVIDER>
struct std::hash<ghassanpl::symbol_base<SYMBOL_PROVIDER>> {
	size_t operator()(const ghassanpl::symbol_base<SYMBOL_PROVIDER>& x) const noexcept { return x.get_hash(); }
};

namespace ghassanpl
{

	template <typename T>
	concept symbol_provider = 
		std::is_class_v<T> 
		&& requires {
			typename T::internal_value_type;
			typename T::hash_type;
			{ T::empty_value() } noexcept -> std::same_as<typename T::internal_value_type>;
			{ T::insert(std::string_view{}) } -> std::same_as<typename T::internal_value_type>;
			{ T::string_for(typename T::internal_value_type{}) } noexcept -> std::same_as<std::string_view>;
			{ T::hash_for(typename T::internal_value_type{}) } noexcept -> std::same_as<typename T::hash_type>;
		}
		&& std::three_way_comparable<typename T::internal_value_type>
		&& std::regular<typename T::internal_value_type>
	;

	template <typename TAG = void>
	struct default_symbol_provider_t
	{
		static default_symbol_provider_t& instance() noexcept
		{
			static default_symbol_provider_t inst;
			return inst;
		}

		using internal_value_type = std::string const*;
		using hash_type = size_t;
		static internal_value_type empty_value() noexcept { return instance().m_empty_string; }
		static internal_value_type insert(std::string_view val)
		{
			if (val.empty())
				return empty_value();

			auto& values = instance().m_values;
			if (auto v = values.find(val); v == values.end())
				return &*values.insert(std::string{ val }).first;
			else
				return &*v;
		}
		static std::string_view string_for(internal_value_type val) noexcept { return val ? std::string_view{ *val } : std::string_view{}; }
		static hash_type hash_for(internal_value_type val) noexcept { return std::hash<const void*>{}(val); }

		/// Utility functions

		void clear() noexcept
		{
			m_values.clear();
			m_values.insert(std::string{});
			m_empty_string = &*m_values.begin();
		}

		size_t size() const noexcept { return m_values.size(); }
		size_t count() const noexcept { return size(); }

		auto const& values() const noexcept { return m_values; }
		auto empty_string() const noexcept { return m_empty_string; }

	protected:

		std::set<std::string, std::less<>> m_values{ std::string{} };
		std::string const* m_empty_string = &*m_values.begin(); /// TODO: Could be made atomic
	};

	using default_symbol_provider = default_symbol_provider_t<void>;
	using symbol = symbol_base<default_symbol_provider>;
}

/// TODO: ostream << and formatter, or enable stringification
/// TODO: a thread-safe version of the symbol provider
