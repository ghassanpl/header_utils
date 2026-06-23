/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp20.h" /// TODO: This could be made compliant with C++17, but I'm lazy (thanks, Copilot)
#include <string>
#include <string_view>
#include <set>
#include <format>

namespace ghassanpl
{
	/// \defgroup Symbol Symbol
	/// Interned strings; see description for details.
	/// 
	/// The `symbol` is a classic interned-string type that enables small storage sizes, content deduplication and quick comparisons,
	/// by only storing the (unique) pointer to a single instance of the string data.
	/// The specific operations of interning, hashing and comparisons are provided by a `symbol_provider` type parameter.
	/// @{

	/// The base class for all symbols
	template <typename SYMBOL_PROVIDER>
	struct symbol_base
	{
		using symbol_provider = SYMBOL_PROVIDER;
		using internal_value_type = typename symbol_provider::internal_value_type;
		using hash_type = typename symbol_provider::hash_type;

		internal_value_type value = symbol_provider::empty_value();

		template <typename T>
		requires std::constructible_from<std::string_view, T> && (!std::same_as<std::remove_cvref_t<T>, symbol_base>)
		symbol_base(T&& val) : value{ symbol_provider::insert(std::forward<T>(val)) } { }

		template <typename T>
		requires std::constructible_from<std::string_view, T> && (!std::same_as<std::remove_cvref_t<T>, symbol_base>)
		symbol_base& operator=(T&& val) { value = symbol_provider::insert(std::forward<T>(val)); return *this; }

		symbol_base() noexcept = default;

		symbol_base(symbol_base const& other) noexcept = default;
		symbol_base(symbol_base&& other) noexcept = default;
		symbol_base& operator=(symbol_base const& other) noexcept = default;
		symbol_base& operator=(symbol_base&& other) noexcept = default;

		[[nodiscard]] hash_type get_hash() const noexcept { return symbol_provider::hash_for(value); }
		[[nodiscard]] std::string_view get_string() const noexcept { return symbol_provider::string_for(value); }
		[[nodiscard]] operator std::string_view() const noexcept { return symbol_provider::string_for(value); }
		[[nodiscard]] explicit operator std::string() const { return std::string{ get_string() }; }
		[[nodiscard]] auto to_string() const noexcept { return std::string{ get_string() }; }

		/// Only available if the internal value type is a pointer
		[[nodiscard]] auto operator->() const noexcept 
		requires requires { { std::to_address(value) } -> meets<std::is_pointer>; } /// Works for std::shared_ptr where std::is_pointer_v would not
		{ return std::to_address(value); }

		[[nodiscard]] bool operator==(symbol_base const& other) const noexcept { return value == other.value || symbol_provider::compare(value, other.value) == 0; }
		[[nodiscard]] auto operator<=>(symbol_base const& other) const noexcept { return symbol_provider::compare(value, other.value); }

		[[nodiscard]] friend bool operator==(std::string_view a, symbol_base const& b) noexcept { return a == b.get_string(); }
		[[nodiscard]] friend auto operator<=>(std::string_view a, symbol_base const& b) noexcept { return a <=> b.get_string(); }

		//[[nodiscard]] auto begin() const noexcept { return get_string().begin(); }
		//[[nodiscard]] auto end() const noexcept { return get_string().end(); }
		[[nodiscard]] bool empty() const noexcept { return value == symbol_provider::empty_value(); }
		[[nodiscard]] size_t size() const noexcept { return get_string().size(); }
		friend std::ostream& operator<<(std::ostream& o, const symbol_base& ptr)
		{
			o << ptr.get_string();
			return o;
		}
	};
	
	/// The requirements for a symbol provider.
	template <typename T>
	concept symbol_provider = 
		std::is_class_v<T> 
		&& requires {
			/// What will be stored inside a symbol
			typename T::internal_value_type;

			/// Usually the same as for std::hash
			typename T::hash_type;

			/// Returns an internal_value_type representing an empty string ("" or std::string{})
			{ T::empty_value() } noexcept -> std::same_as<typename T::internal_value_type>;

			/// Called when we want to intern a string
			{ T::insert(std::string_view{}) } -> std::same_as<typename T::internal_value_type>;

			/// Returns the string the internal value represents
			{ T::string_for(typename T::internal_value_type{}) } noexcept -> std::same_as<std::string_view>;

			/// Could hash the string or just the pointer to it, depends on which behavior you prefer
			{ T::hash_for(typename T::internal_value_type{}) } noexcept -> std::same_as<typename T::hash_type>;

			/// Compares the symbols by their internal value; this could be a value-comparison if lexicographical ordering of strings is important to you
			{ T::compare(typename T::internal_value_type{}, typename T::internal_value_type{}) } noexcept -> std::same_as<std::strong_ordering>;
		}
		&& std::three_way_comparable<typename T::internal_value_type>
		&& std::regular<typename T::internal_value_type>
	;

	/// @}
}

template <typename SYMBOL_PROVIDER>
struct std::hash<ghassanpl::symbol_base<SYMBOL_PROVIDER>> {
	size_t operator()(const ghassanpl::symbol_base<SYMBOL_PROVIDER>& x) const noexcept { return x.get_hash(); }
};

template <typename SYMBOL_PROVIDER>
struct std::formatter<ghassanpl::symbol_base<SYMBOL_PROVIDER>> : std::formatter<std::string_view, char> {
	template <typename FORMAT_CONTEXT>
	auto format(ghassanpl::symbol_base<SYMBOL_PROVIDER> const& val, FORMAT_CONTEXT& ctx) const {
		return std::formatter<std::string_view, char>::format(val.get_string(), ctx);
	}
};

namespace ghassanpl
{

	/// The default symbol provider usable for most symbol implementations. NOT THREAD SAFE. Uses a `TAG` type parameter if you want to create
	/// different "namespaces" of symbols.
	/// 
	/// This class can be easily made thread-safe, just wrap all operations that touch `m_values` in a mutex (specifically `insert` and `clear`).
	/// \ingroup Symbol
	template <typename TAG = void>
	struct default_symbol_provider_t
	{
		default_symbol_provider_t() = default;
		default_symbol_provider_t(default_symbol_provider_t const&) = delete;
		default_symbol_provider_t& operator=(default_symbol_provider_t const&) = delete;

		static default_symbol_provider_t& instance() noexcept
		{
			static default_symbol_provider_t inst;
			return inst;
		}

		using internal_value_type = std::string const*;
		using hash_type = size_t;
		[[nodiscard]] static internal_value_type empty_value() noexcept { return instance().m_empty_string; }
		[[nodiscard]] static internal_value_type insert(std::string_view val)
		{
			if (val.empty())
				return empty_value();

			auto& values = instance().m_values;
			if (auto v = values.find(val); v == values.end())
				return &*values.insert(std::string{ val }).first;
			else
				return &*v;
		}
		[[nodiscard]] static std::string_view string_for(internal_value_type val) noexcept { return val ? std::string_view{ *val } : std::string_view{}; }
		[[nodiscard]] static hash_type hash_for(internal_value_type val) noexcept { return std::hash<const void*>{}(val); }

		[[nodiscard]] static std::strong_ordering compare(internal_value_type a, internal_value_type b) noexcept {
			return (a == b) ? std::strong_ordering::equal : (*a <=> *b);
		}

		// Utility functions

		void clear() noexcept
		{
			/// Extracting preserves the pointer to the empty string, which means we don't have to change m_empty_string
			auto e = m_values.extract(std::string{});
			m_values.clear();
			m_values.insert(std::move(e));
		}

		[[nodiscard]] size_t size() const noexcept { return m_values.size(); }
		[[nodiscard]] size_t count() const noexcept { return size(); }

		[[nodiscard]] auto const& values() const noexcept { return m_values; }
		[[nodiscard]] auto empty_string() const noexcept { return m_empty_string; }

	protected:

		std::set<std::string, std::less<>> m_values{ std::string{} };
		std::string const* m_empty_string = &*m_values.begin();
	};

	/// A basic `symbol_provider` suitable for most single-threaded uses.
	/// \ingroup Symbol
	using default_symbol_provider = default_symbol_provider_t<void>;

	/// A `symbol` type suitable for most single-threaded uses.
	/// \ingroup Symbol
	using symbol = symbol_base<default_symbol_provider>;
}

/// TODO: ostream << and formatter, or enable stringification
/// TODO: thread-safe/thread-local versions of the symbol provider
