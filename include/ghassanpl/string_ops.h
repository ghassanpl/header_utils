/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>
#include <algorithm>
#include <vector>
#include <sstream>
#include <charconv>
#include <variant>
#include <optional>
#include <ranges>

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif
#if !defined(__cpp_lib_ranges)
#error "This library requires ranges"
#endif
#if defined(__cpp_lib_format)
#include <format>
#endif
#if !defined(__cpp_lib_to_address)
#error "This library requires std::to_address"
#endif

/// Adds a few utility functions that deal with strings and string_views.


namespace ghassanpl::string_ops
{
	/// ///////////////////////////// ///
	/// Useful concepts
	/// ///////////////////////////// ///
	
	template <typename T, typename CHAR_TYPE = char>
	concept string_or_char = std::is_constructible_v<std::basic_string_view<CHAR_TYPE>, T> || std::is_constructible_v<CHAR_TYPE, T>;

	template <typename T, typename CHAR_TYPE = char>
	concept stringable = (std::ranges::range<T> && std::is_convertible_v<std::ranges::range_value_t<T>, CHAR_TYPE>);


	template <typename T>
	concept string8 = std::same_as<T, std::string> || std::same_as<T, std::u8string>;
	template <typename T>
	concept stringable8 = std::convertible_to<T, std::string_view> || std::convertible_to<T, std::u8string_view>;
	template <typename T>
	concept string_view8 = std::same_as<T, std::string_view> || std::same_as<T, std::u8string_view>;

	template <typename T>
	concept string16 = (sizeof(wchar_t) == sizeof(char16_t) && std::same_as<T, std::wstring>) || std::same_as<T, std::u16string>;
	template <typename T>
	concept stringable16 = (sizeof(wchar_t) == sizeof(char16_t) && std::convertible_to<T, std::wstring_view>) || std::convertible_to<T, std::u16string_view>;
	template <typename T>
	concept string_view16 = (sizeof(wchar_t) == sizeof(char16_t) && std::same_as<T, std::wstring_view>) || std::same_as<T, std::u16string_view>;

	template <typename T>
	concept string32 = (sizeof(wchar_t) == sizeof(char32_t) && std::same_as<T, std::wstring>) || std::same_as<T, std::u32string>;
	template <typename T>
	concept stringable32 = (sizeof(wchar_t) == sizeof(char32_t) && std::convertible_to<T, std::wstring_view>) || std::convertible_to<T, std::u32string_view>;
	template <typename T>
	concept string_view32 = (sizeof(wchar_t) == sizeof(char32_t) && std::same_as<T, std::wstring_view>) || std::same_as<T, std::u32string_view>;

	/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///
	/// Makes
	/// 
	/// Rationale: Even though C++20 has a range constructor, it uses `operator-` on its arguments, which means that iterators from disparate string_views do not work, even
	/// if they point to a contiguous string range (for example, were made from substrings of the same string_view). Hence these functions.
	/// They should work with any pair of iterators (no support for sentinels yet, unfortunately), support nulls, and are almost as strong as the string_view range constructor
	/// in terms of exception and type safety. As with the respective constructors, undefined behavior when `start` > `end`.
	/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///

	[[nodiscard]] constexpr inline std::string_view make_sv(nullptr_t start, nullptr_t end) { return std::string_view{}; }
	
	template <std::contiguous_iterator IT, std::contiguous_iterator IT2>
	requires std::is_same_v<std::iter_value_t<IT>, char> && std::is_same_v<std::iter_value_t<IT2>, char>
	[[nodiscard]] constexpr inline std::string_view make_sv(IT start, IT2 end) noexcept(noexcept(std::to_address(start))) { return std::string_view{ std::to_address(start), static_cast<size_t>(std::to_address(end) - std::to_address(start)) }; }

	[[nodiscard]] constexpr inline std::string_view make_sv(char& single_char) noexcept { return make_sv(&single_char, &single_char + 1); }
	[[nodiscard]] constexpr inline std::string_view make_sv(const char* str) noexcept { return str ? std::string_view{ str } : std::string_view{}; }
	[[nodiscard]] constexpr inline std::string_view make_sv(const unsigned char* str) noexcept { return str ? std::string_view{ (const char*)str } : std::string_view{}; }
	[[nodiscard]] constexpr inline std::wstring_view make_sv(const wchar_t* str) noexcept { return str ? std::wstring_view{ str } : std::wstring_view{}; }

	template <typename C>
	[[nodiscard]] constexpr inline std::basic_string_view<C> make_sv(std::basic_string_view<C> id) noexcept { return id; }
	template <typename C>
	[[nodiscard]] constexpr inline std::basic_string_view<C> make_sv(std::basic_string<C> const& id) noexcept { return id; }

	[[nodiscard]] constexpr inline std::string make_string(nullptr_t start, nullptr_t end) { return std::string{}; }

	template <std::contiguous_iterator IT, std::contiguous_iterator IT2>
	requires std::is_same_v<std::iter_value_t<IT>, char>&& std::is_same_v<std::iter_value_t<IT2>, char>
	[[nodiscard]] inline std::string make_string(IT start, IT2 end) noexcept(noexcept(std::to_address(start))) { return std::string{ ::ghassanpl::string_ops::make_sv(start, end) }; }

	/// for predicates
	[[nodiscard]] inline std::string to_string(std::string_view from) noexcept { return std::string{ from }; }

	[[nodiscard]] inline std::string to_string(std::u8string_view from) noexcept { return std::string{ from.data(), from.data() + from.size() }; }

	template<typename T>
	[[nodiscard]] inline std::string to_string(T const& t) requires requires { std::to_string(t); } { return std::to_string(t); }

	[[nodiscard]] constexpr inline std::string const& to_string(std::same_as<std::string> auto const& s) { return s; }

	template<typename T>
	[[nodiscard]] inline std::string to_string(std::optional<T> const& o) { if (o.has_value()) return std::to_string(o.value()); return "(empty)"; }

	/// ///////////////////////////// ///
	/// Other string_view utils
	/// ///////////////////////////// ///

	[[nodiscard]] constexpr inline std::string_view back(std::string_view child_to_back_up, std::string_view parent, size_t n = 1) noexcept
	{
		return make_sv(std::max(child_to_back_up.data() - n, parent.data()), child_to_back_up.data() + child_to_back_up.size());
	}

	[[nodiscard]] constexpr inline std::string_view back(std::string_view child_to_back_up, size_t n = 1) noexcept
	{
		return make_sv(child_to_back_up.data() - n, child_to_back_up.data() + child_to_back_up.size());
	}

	/// ///////////////////////////// ///
	/// ASCII functions
	/// ///////////////////////////// ///

	namespace ascii
	{
#if 0
		constexpr inline uint64_t is_flag_set(std::array<uint64_t, 2> flags, char32_t cp) noexcept
		{
			const auto flag_el = (cp >> 6) & 1;
			const auto bit = uint64_t(1) << (cp & 63);
			return (flags[flag_el] & bit) != 0;
		}
		
		template <uint64_t HIGH, uint64_t LOW>
		constexpr inline bool is_flag_set(char32_t cp) noexcept
		{
			const auto low_mask = uint64_t(0) - (cp >> 6 == 0);
			const auto high_mask = uint64_t(0) - (cp >> 6 == 1);
			const auto bit = uint64_t(1) << (cp & 63);

			return ((bit & low_mask) & LOW) or ((bit & high_mask) & HIGH);
		}

		constexpr inline std::array<uint64_t, 2> get_flags_for(std::string_view str) noexcept
		{
			std::array<uint64_t, 2> result{0,0};
			for (auto c : str)
				result[(char32_t(c) >> 6) & 1] |= (char32_t(c) & 63);
			return result;
		}

		template <typename FUNC>
		constexpr inline std::array<uint64_t, 2> get_flags_for(FUNC&& pred) noexcept
		{
			std::array<uint64_t, 2> result{0,0};
			for (char32_t c=0; c<128; ++c)
				if (pred(c))
					result[(char32_t(c) >> 6) & 1] |= (char32_t(c) & 63);
			return result;
		}
#endif

		/// Our own versions of <cctype> functions that do not block, are defined for values outside of uint8_t, and do not depend on locale.
		/// RATIONALE: We are using numbers (e.g. 65) instead of character literals (e.g. 'A'), because the encoding of this source file might not be ASCII-based
		/// Thanks to @fmatthew5876 for inspiration

		[[nodiscard]] constexpr inline bool isalpha(char32_t cp) noexcept { return (cp >= 65 && cp <= 90) || (cp >= 97 && cp <= 122); }
		[[nodiscard]] constexpr inline bool isdigit(char32_t cp) noexcept { return cp >= 48 && cp <= 57; }
		[[nodiscard]] constexpr inline bool isodigit(char32_t cp) noexcept { return cp >= 48 && cp <= 55; }
		[[nodiscard]] constexpr inline bool isxdigit(char32_t d) noexcept { return (d >= 48 && d <= 57) || (d >= 65 && d <= 70) || (d >= 97 && d <= 102); }
		[[nodiscard]] constexpr inline bool isalnum(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isdigit(cp) || ::ghassanpl::string_ops::ascii::isalpha(cp); }
		[[nodiscard]] constexpr inline bool isident(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isdigit(cp) || ::ghassanpl::string_ops::ascii::isalpha(cp) || cp == 95; }
		[[nodiscard]] constexpr inline bool isidentstart(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isalpha(cp) || cp == 95; }
		[[nodiscard]] constexpr inline bool isspace(char32_t cp) noexcept { return (cp >= 9 && cp <= 13) || cp == 32; }
		[[nodiscard]] constexpr inline bool ispunct(char32_t cp) noexcept { return (cp >= 33 && cp <= 47) || (cp >= 58 && cp <= 64) || (cp >= 91 && cp <= 96) || (cp >= 123 && cp <= 126); }
		[[nodiscard]] constexpr inline bool islower(char32_t cp) noexcept { return cp >= 97 && cp <= 122; }
		[[nodiscard]] constexpr inline bool isupper(char32_t cp) noexcept { return cp >= 65 && cp <= 90; }
		[[nodiscard]] constexpr inline bool iscntrl(char32_t cp) noexcept { return cp == 0x7F || cp < 0x20; }
		[[nodiscard]] constexpr inline bool isblank(char32_t cp) noexcept { return cp == 32 || cp == 9; }
		[[nodiscard]] constexpr inline bool isgraph(char32_t cp) noexcept { return cp >= 33 && cp <= 126; }
		[[nodiscard]] constexpr inline bool isprint(char32_t cp) noexcept { return cp >= 32 && cp <= 126; }

		[[nodiscard]] constexpr inline bool isany(char32_t cp, std::string_view chars) noexcept { return cp < 128 && std::find(chars.begin(), chars.end(), (char)cp) != chars.end(); }

		[[nodiscard]] constexpr inline char32_t toupper(char32_t cp) noexcept { 
#if 0
			return (CharType)(ToUnsigned(Char) - ((uint32(Char) - 'a' < 26u) << 5));
#endif
			return (cp >= 97 && cp <= 122) ? (cp ^ 0b100000) : cp; 
		}
		[[nodiscard]] constexpr inline char32_t tolower(char32_t cp) noexcept { 
#if 0
			return (CharType)(ToUnsigned(Char) + ((uint32(Char) - 'A' < 26u) << 5));
#endif
			return (cp >= 65 && cp <= 90)  ? (cp | 0b100000) : cp; 
		}

		/// ASCII-string-based utilities that make use of the above functions

		template <stringable T>
		[[nodiscard]] constexpr inline std::string tolower(T const& str) noexcept {
			using std::ranges::begin;
			using std::ranges::end;
			std::string result;
			if constexpr (std::ranges::sized_range<T>)
				result.reserve(std::ranges::size(str));
			std::transform(begin(str), end(str), std::back_inserter(result), [](char cp) { return (char)::ghassanpl::string_ops::ascii::tolower(cp); });
			return result;
		}

		[[nodiscard]] constexpr inline std::string tolower(std::string str) noexcept {
			std::transform(begin(str), end(str), begin(str), [](char cp) { return (char)::ghassanpl::string_ops::ascii::tolower(cp); });
			return str;
		}

		[[nodiscard]] constexpr inline std::string tolower(const char* str) noexcept {
			if (str)
				return tolower(std::string{str});
			return {};
		}

		/*
		//[[nodiscard]] inline std::string tolower(std::string_view str) noexcept { return ::ghassanpl::string_ops::ascii::tolower(std::string{str}); }
		//[[nodiscard]] inline std::string tolower(const char* str) noexcept { return ::ghassanpl::string_ops::ascii::tolower(make_sv(str)); }

		[[nodiscard]] inline std::string toupper(std::string str) noexcept { std::for_each(str.begin(), str.end(), [](char& cp) { cp = (char)::ghassanpl::string_ops::ascii::toupper(cp); }); return str; }
		[[nodiscard]] inline std::string toupper(std::string_view str) noexcept { return ::ghassanpl::string_ops::ascii::toupper(std::string{str}); }
		[[nodiscard]] inline std::string toupper(const char* str) noexcept { return ::ghassanpl::string_ops::ascii::toupper(make_sv(str)); }
		*/
		template <stringable T>
		[[nodiscard]] inline std::string toupper(T const& str) noexcept {
			using std::ranges::begin;
			using std::ranges::end;
			std::string result;
			if constexpr (std::ranges::sized_range<T>)
				result.reserve(std::ranges::size(str));
			std::transform(begin(str), end(str), std::back_inserter(result), [](char cp) { return (char)::ghassanpl::string_ops::ascii::toupper(cp); });
			return result;
		}

		[[nodiscard]] inline std::string toupper(std::string str) noexcept {
			std::transform(begin(str), end(str), begin(str), [](char cp) { return (char)::ghassanpl::string_ops::ascii::toupper(cp); });
			return str;
		}

		[[nodiscard]] inline std::string toupper(const char* str) noexcept {
			if (str)
				return toupper(std::string{ str });
			return {};
		}

		/// Convert a number between 0 and 9/15 to its ASCII representation (only gives meaningful results with arguments between 0 and 9/15)

		[[nodiscard]] constexpr inline char32_t number_to_digit(int v) noexcept { return char32_t(v) + 48; }
		[[nodiscard]] constexpr inline char32_t number_to_xdigit(int v) noexcept { return (v > 9) ? (char32_t(v - 10) + 65) : (char32_t(v) + 48); }

		/// Convert an ASCII (x)digit to its numerical value (only gives meaningful results with valid (x)digit arguments)

		[[nodiscard]] constexpr inline int digit_to_number(char32_t cp) noexcept { return int(cp - 48); }
		[[nodiscard]] constexpr inline int xdigit_to_number(char32_t cp) noexcept { return (cp >= 97 && cp <= 102) ? int(cp - 97) : int((cp >= 65 && cp <= 70) ? (cp - 55) : (cp - 48)); }

		/// Case-invariant comparisons and sorts

		[[nodiscard]] constexpr bool strings_equal_ignore_case(std::string_view a, std::string_view b)
		{
			return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return ::ghassanpl::string_ops::ascii::toupper(a) == ::ghassanpl::string_ops::ascii::toupper(b); });
		}

		[[nodiscard]] constexpr bool string_starts_with_ignore_case(std::string_view a, std::string_view b)
		{
			return strings_equal_ignore_case(a.substr(0, b.size()), b);
		}

		[[nodiscard]] constexpr bool string_ends_with_ignore_case(std::string_view a, std::string_view b)
		{
			if (b.size() > a.size()) return false;
			return strings_equal_ignore_case(a.substr(a.size() - b.size()), b);
		}

		[[nodiscard]] constexpr auto string_find_ignore_case(std::string_view a, std::string_view b)
		{
			return std::search(
				a.begin(), a.end(),
				b.begin(), b.end(),
				[](char ch1, char ch2) { return ::ghassanpl::string_ops::ascii::tolower(ch1) == ::ghassanpl::string_ops::ascii::tolower(ch2); }
			);
		}

		[[nodiscard]] constexpr auto string_find_last_ignore_case(std::string_view a, std::string_view b)
		{
			return std::find_end(
				a.begin(), a.end(),
				b.begin(), b.end(),
				[](char ch1, char ch2) { return ::ghassanpl::string_ops::ascii::tolower(ch1) == ::ghassanpl::string_ops::ascii::tolower(ch2); }
			);
		}

		[[nodiscard]] constexpr auto string_contains_ignore_case(std::string_view a, std::string_view b)
		{
			return string_find_ignore_case(a, b) != a.end();
		}

		[[nodiscard]] constexpr bool lexicographical_compare_ignore_case(std::string_view a, std::string_view b)
		{
			return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return ::ghassanpl::string_ops::ascii::toupper(a) < ::ghassanpl::string_ops::ascii::toupper(b); });
		}

		[[nodiscard]] constexpr auto lexicographical_compare_ignore_case_three_way(std::string_view a, std::string_view b)
		{
			return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return ::ghassanpl::string_ops::ascii::toupper(a) <=> ::ghassanpl::string_ops::ascii::toupper(b); });
		}

		struct string_view_case_insensitive
		{
			std::string_view value;

			friend constexpr bool operator ==(std::string_view a, string_view_case_insensitive b) noexcept { return strings_equal_ignore_case(a, b.value); }
			friend constexpr auto operator <=>(std::string_view a, string_view_case_insensitive b) noexcept { return lexicographical_compare_ignore_case_three_way(a, b.value); }

			constexpr bool operator ==(std::string_view a) const noexcept { return strings_equal_ignore_case(value, a); }
			constexpr auto operator <=>(std::string_view a) const noexcept { return lexicographical_compare_ignore_case_three_way(value, a); }

			constexpr bool operator ==(string_view_case_insensitive const& other) const noexcept { return strings_equal_ignore_case(value, other.value); }
			constexpr auto operator <=>(string_view_case_insensitive const& other) const noexcept { return lexicographical_compare_ignore_case_three_way(value, other.value); }
		};

		constexpr inline string_view_case_insensitive operator"" _i(const char* str, size_t size) noexcept { return string_view_case_insensitive{ std::string_view{str, str + size} }; }
	}

#pragma push_macro("isascii")
#undef isascii
	[[nodiscard]] constexpr inline bool isascii(char32_t cp) noexcept { return cp <= 127; }
#pragma pop_macro("isascii")

	/// ///////////////////////////// ///
	/// Pre-C++23 stuff
	/// ///////////////////////////// ///
	
	[[nodiscard]] inline bool contains(std::string_view str, char c)
	{
#if __cpp_lib_string_contains
		return str.contains(c);
#else
		return ::std::find(str.begin(), str.end(), c) != str.end();
#endif
	}

	/// ///////////////////////////// ///
	/// Trims
	/// ///////////////////////////// ///

	[[nodiscard]] constexpr inline std::string_view trimmed_whitespace_right(std::string_view str) noexcept { return make_sv(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base()); }
	[[nodiscard]] constexpr inline std::string_view trimmed_whitespace_left(std::string_view str) noexcept { return make_sv(std::find_if_not(str.begin(), str.end(), ::ghassanpl::string_ops::ascii::isspace), str.end()); }
	[[nodiscard]] constexpr inline std::string_view trimmed_whitespace(std::string_view str) noexcept { return trimmed_whitespace_left(trimmed_whitespace_right(str)); }
	[[nodiscard]] constexpr inline std::string_view trimmed_until(std::string_view str, char chr) noexcept { return make_sv(std::find(str.begin(), str.end(), chr), str.end()); }
	[[nodiscard]] constexpr inline std::string_view trimmed(std::string_view str, char chr) noexcept { return make_sv(std::find_if_not(str.begin(), str.end(), [chr](char c) { return c == chr; }), str.end()); }

	[[nodiscard]] inline std::string trimmed_whitespace_right(std::string&& str) noexcept { str.erase(std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base(), str.end()); return str; }
	[[nodiscard]] inline std::string trimmed_whitespace_left(std::string&& str) noexcept { str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), ::ghassanpl::string_ops::ascii::isspace)); return str; }
	[[nodiscard]] inline std::string trimmed_whitespace(std::string&& str) noexcept { return trimmed_whitespace_left(trimmed_whitespace_right(std::move(str))); }
	[[nodiscard]] inline std::string trimmed_until(std::string&& str, char chr) noexcept { str.erase(str.begin(), std::find(str.begin(), str.end(), chr)); return str; }
	[[nodiscard]] inline std::string trimmed(std::string&& str, char chr) noexcept { str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), [chr](char c) { return c == chr; })); return str; }
	//[[nodiscard]] inline std::string_view trimmed(std::string_view str) noexcept { if (!str.empty()) str.remove_prefix(1); return str; }
	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	[[nodiscard]] inline std::string_view trimmed_while(std::string_view str, FUNC&& func) noexcept { return ::ghassanpl::string_ops::make_sv(std::find_if_not(str.begin(), str.end(), std::forward<FUNC>(func)), str.end()); }

	constexpr inline void trim_whitespace_right(std::string_view& str) noexcept { str = make_sv(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base()); }
	constexpr inline void trim_whitespace_left(std::string_view& str) noexcept { str = make_sv(std::find_if_not(str.begin(), str.end(), ::ghassanpl::string_ops::ascii::isspace), str.end()); }
	constexpr inline void trim_whitespace(std::string_view& str) noexcept { trim_whitespace_left(str); trim_whitespace_right(str); }
	constexpr inline void trim_until(std::string_view& str, char chr) noexcept { str = trimmed_until(str, chr); }
	constexpr inline void trim(std::string_view& str, char chr) noexcept { str = trimmed(str, chr); }
	//inline void trim(std::string_view& str) noexcept { if (!str.empty()) str.remove_prefix(1); }
	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	constexpr void trim_while(std::string_view& str, FUNC&& func) noexcept { str = trimmed_while(str, std::forward<FUNC>(func)); }

	/// ///////////////////////////// ///
	/// Consume
	/// ///////////////////////////// ///
	
	[[nodiscard]] inline char consume(std::string_view& str)
	{
		if (str.empty())
			return {};
		const auto result = str[0];
		str.remove_prefix(1);
		return result;
	}

	[[nodiscard]] inline bool consume(std::string_view& str, char val)
	{
		if (str.starts_with(val))
		{
			str.remove_prefix(1);
			return true;
		}
		return false;
	}

	[[nodiscard]] inline bool consume(std::string_view& str, std::string_view val)
	{
		if (str.starts_with(val))
		{
			str.remove_prefix(val.size());
			return true;
		}
		return false;
	}

	template <typename PRED>
	requires std::is_invocable_r_v<bool, PRED, char>
	[[nodiscard]] inline char consume(std::string_view& str, PRED&& pred)
	{
		if (!str.empty() && pred(str[0]))
		{
			const auto result = str[0];
			str.remove_prefix(1);
			return result;
		}
		return {};
	}

	[[nodiscard]] inline bool consume_at_end(std::string_view& str, char val)
	{
		if (str.ends_with(val))
		{
			str.remove_prefix(1);
			return true;
		}
		return false;
	}

	[[nodiscard]] inline bool consume_at_end(std::string_view& str, std::string_view val)
	{
		if (str.ends_with(val))
		{
			str.remove_suffix(val.size());
			return true;
		}
		return false;
	}

	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	[[nodiscard]] inline std::string_view consume_while(std::string_view& str, FUNC&& pred)
	{
		const auto start = str.begin();
		while (!str.empty() && pred(str[0]))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	[[nodiscard]] inline std::string_view consume_while(std::string_view& str, char c)
	{
		const auto start = str.begin();
		while (str.starts_with(c))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}
	
	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	[[nodiscard]] inline std::string_view consume_until(std::string_view& str, FUNC&& pred)
	{
		const auto start = str.begin();
		while (!str.empty() && !pred(str[0]))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	[[nodiscard]] inline std::string_view consume_until(std::string_view& str, char c)
	{
		const auto start = str.begin();
		while (!str.empty() && str[0] != c)
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	[[nodiscard]] inline std::string_view consume_until(std::string_view& str, std::string_view end)
	{
		auto it = std::search(str.begin(), str.end(), end.begin(), end.end());
		auto result = make_sv(str.begin(), it);
		str = { it, str.end() };
		return result;
	}

	[[nodiscard]] inline std::string_view consume_n(std::string_view& str, size_t n)
	{
		n = std::min(str.size(), n);
		auto result = str.substr(0, n);
		str.remove_prefix(n);
		return result;
	}

	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	[[nodiscard]] inline std::string_view consume_n(std::string_view& str, size_t n, FUNC&& pred)
	{
		n = std::min(str.size(), n);
		const auto start = str.begin();
		while (n-- && !str.empty() && pred(str[0]))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	template <typename CALLBACK>
	requires std::is_invocable_r_v<bool, CALLBACK, std::string_view&>
	[[nodiscard]] inline bool consume_delimited_list_non_empty(std::string_view& str, std::string_view delimiter, CALLBACK callback)
	{
		do
		{
			trim_whitespace_left(str);
			if (!callback(str))
				return false;
			trim_whitespace_left(str);
		} while (consume(str, delimiter));
		return true;
	}

	template <typename CALLBACK>
	requires std::is_invocable_r_v<bool, CALLBACK, std::string_view>
	[[nodiscard]] inline bool consume_delimited_list(std::string_view& str, std::string_view delimiter, std::string_view closer, CALLBACK callback)
	{
		trim_whitespace_left(str);
		while (!str.empty())
		{
			trim_whitespace_left(str);
			if (!callback(str))
				return false;
			trim_whitespace_left(str);
			if (!consume(str, delimiter))
				return consume(str, closer);
		}
		return false;
	}

	[[nodiscard]] inline std::string_view consume_c_identifier(std::string_view& str)
	{
		if (str.empty() || !ascii::isidentstart(str[0]))
			return {};

		const auto start = str.begin();
		str.remove_prefix(1);
		trim_while(str, ascii::isident);
		return make_sv(start, str.begin());
	}

	[[nodiscard]] inline std::string_view consume_c_identifier_with(std::string_view& str, std::string_view additional_chars)
	{
		if (str.empty() || !(ascii::isidentstart(str[0]) || contains(additional_chars, str[0])))
			return {};

		const auto start = str.begin();
		str.remove_prefix(1);
		trim_while(str, [additional_chars](char c) { return ascii::isident(c) || contains(additional_chars, c); });
		return make_sv(start, str.begin());
	}


#if __cpp_lib_to_chars
	[[nodiscard]] inline std::pair<std::string_view, double> consume_c_float(std::string_view& str)
	{
		if (str.empty() || !(ascii::isdigit(str[0]) || str[0] == '-'))
			return {};

		std::pair<std::string_view, double> result;

		const auto from_chars_result = std::from_chars(str.data(), str.data() + str.size(), result.second);
		if (from_chars_result.ec != std::errc{})
			return { {}, std::numeric_limits<double>::quiet_NaN() };

		result.first = make_sv(str.data(), from_chars_result.ptr);
		str.remove_prefix(result.first.size());
		return result;
	}

	[[nodiscard]] inline std::pair<std::string_view, int64_t> consume_c_integer(std::string_view& str, int base = 10)
	{
		if (str.empty() || !(ascii::isdigit(str[0]) || str[0] == '-'))
			return {};

		std::pair<std::string_view, int64_t> result;

		const auto from_chars_result = std::from_chars(str.data(), str.data() + str.size(), result.second, base);
		if (from_chars_result.ec != std::errc{})
			return { {}, 0 };

		result.first = make_sv(str.data(), from_chars_result.ptr);
		str.remove_prefix(result.first.size());
		return result;
	}

	[[nodiscard]] inline std::pair<std::string_view, uint64_t> consume_c_unsigned(std::string_view& str, int base = 10)
	{
		if (str.empty() || !ascii::isdigit(str[0]))
			return {};

		std::pair<std::string_view, uint64_t> result;

		const auto from_chars_result = std::from_chars(str.data(), str.data() + str.size(), result.second, base);
		if (from_chars_result.ec != std::errc{})
			return { {}, 0 };

		result.first = make_sv(str.data(), from_chars_result.ptr);
		str.remove_prefix(result.first.size());
		return result;
	}

	size_t append_utf8(string8 auto& buffer, char32_t cp);

	template <char DELIMITER = '\''>
	[[nodiscard]] inline std::pair<std::string_view, std::string> consume_c_string(std::string_view& strv)
	{
		if (strv.empty() || strv[0] != DELIMITER)
			return {};

		std::pair<std::string_view, std::string> result;

		auto view = strv;
		auto start = view.begin();
		view.remove_prefix(1);
		while (view[0] != DELIMITER)
		{
			auto cp = consume(view);
			if (cp == '\\')
			{
				cp = consume(view);
				if (view.empty())
					return {}; /// Unterminated string literal

				switch (cp)
				{
				case 'n': result.second += '\n'; break;
				case '"': result.second += '"'; break;
				case '\'': result.second += '\''; break;
				case '\\': result.second += '\\'; break;
				case 'b': result.second += '\b'; break;
				case 'r': result.second += '\r'; break;
				case 'f': result.second += '\f'; break;
				case 't': result.second += '\t'; break;
				case '0': result.second += '\0'; break;
				case 'o':
				{
					auto num = consume_n(view, 3);
					if (num.size() < 3 || view.empty()) return {}; /// malformed

					auto parsed = consume_c_integer(num, 8);
					if (parsed.first.empty() || !num.empty()) return {}; /// malformed

					if (parsed.second > 255) return {}; /// invalid octal
					result.second.push_back((char)parsed.second);
					break;
				}
				case 'x':
				{
					auto num = consume_n(view, 2);
					if (num.size() < 2 || view.empty()) return {}; /// malformed

					auto parsed = consume_c_integer(num, 16);
					if (parsed.first.empty() || !num.empty()) return {}; /// malformed

					append_utf8(result.second, (char32_t)parsed.second);
					break;
				}
				case 'u':
				{
					auto num = consume_n(view, 4);
					if (num.size() < 4 || view.empty()) return {}; /// malformed

					auto parsed = consume_c_integer(num, 16);
					if (parsed.first.empty() || !num.empty()) return {}; /// malformed

					append_utf8(result.second, (char32_t)parsed.second);
					break;
				}
				case 'U':
				{
					auto num = consume_n(view, 8);
					if (num.size() < 8 || view.empty()) return {}; /// malformed

					auto parsed = consume_c_integer(num, 16);
					if (parsed.first.empty() || !num.empty()) return {}; /// malformed

					append_utf8(result.second, (char32_t)parsed.second);
					break;
				}
				default:
					return {}; /// unknown escape character
				}
			}
			else
			{
				result.second += cp;
			}

			if (view.empty())
				return {}; /// unterminated
		}

		if (!consume(view, DELIMITER))
			return {}; /// unterminated

		result.first = make_sv(start, view.begin());
		strv = view;
		return result;
	}

	[[nodiscard]] inline std::tuple<std::string_view, std::variant<double, uint64_t, int64_t>> consume_c_number(std::string_view& str)
	{
		if (str.empty())
			return {};

		auto first_char = str[0];
		if (first_char == '-')
		{
			/// We need to parse then complement the integer
			/// TODO: this
			return {};
		}
		else if (consume(str, "0x"))
			return consume_c_unsigned(str, 16);
		else if (consume(str, "0b"))
			return consume_c_unsigned(str, 1);
		else if (consume(str, "0"))
			return consume_c_unsigned(str, 8);
		else if (ascii::isdigit(first_char))
		{
			{
				auto result = consume_c_float(str);
				if (!result.first.empty())
					return result;
			}

			{
				auto result = consume_c_unsigned(str);
				if (!result.first.empty())
					return result;
			}

			{
				auto result = consume_c_integer(str);
				if (!result.first.empty())
					return result;
			}
		}
		return {};
	}

	/// TODO: do this correctly
	[[nodiscard]] inline std::tuple<std::string_view, std::variant<std::string, double, uint64_t, int64_t>> consume_c_literal(std::string_view& str)
	{
		if (str.empty())
			return {};

		auto first_char = str[0];
		if (first_char == '\'')
			return consume_c_string(str);
		else if (first_char == '"')
			return consume_c_string<'"'>(str);

		auto [range, value] = consume_c_number(str);
		if (range.empty()) return {};

		return { range, std::visit([](auto&& v) { return std::variant<std::string, double, uint64_t, int64_t>{std::move(v)}; }, std::move(value)) };
	}

#endif

	/// ///////////////////////////// ///
	/// Other
	/// ///////////////////////////// ///
	
	/// TODO: Move splits to use std::ranges::(lazy_)split_view
	///       But check performance vs our split first!

	template <typename FUNC>
	requires std::is_invocable_v<FUNC, std::string_view, bool>
	constexpr void split(std::string_view source, char delim, FUNC&& func) noexcept(noexcept(func(std::string_view{}, true)))
	{
		size_t next = 0;
		while ((next = source.find_first_of(delim)) != std::string::npos)
		{
			func(source.substr(0, next), false);
			source.remove_prefix(next + 1);
		}
		func(source, true);
	}

	template <typename FUNC>
	requires std::is_invocable_v<FUNC, std::string_view, bool>
	constexpr void split(std::string_view source, std::string_view delim, FUNC&& func) noexcept(noexcept(func(std::string_view{}, true)))
	{
		const size_t delim_size = delim.size();
		if (delim_size == 0) return;

		size_t next = 0;
		while ((next = source.find(delim)) != std::string::npos)
		{
			func(source.substr(0, next), false);
			source.remove_prefix(next + delim_size);
		}
		func(source, true);
	}

	template <typename FUNC>
	requires std::is_invocable_v<FUNC, std::string_view, bool>
	constexpr void split_on_any(std::string_view source, std::string_view delim, FUNC&& func) noexcept(noexcept(func(std::string_view{}, true)))
	{
		if (delim.empty()) return;

		size_t next = 0;
		while ((next = source.find_first_of(delim)) != std::string::npos)
		{
			func(source.substr(0, next), false);
			source.remove_prefix(next + 1);
		}
		func(source, true);
	}

	template <typename DELIM_FUNC, typename FUNC>
	requires std::is_invocable_v<FUNC, std::string_view, bool> && std::is_invocable_r_v<size_t, DELIM_FUNC, std::string_view>
	inline void split_on(std::string_view source, DELIM_FUNC&& delim, FUNC&& func) noexcept(noexcept(func(std::string_view{}, true)) && noexcept(delim(std::string_view{})))
	{
		size_t start = 0;
		size_t end = 0;
		end = delim(source);
		while (end != std::string::npos)
		{
			func(source.substr(start, end-start), false);
			//source.remove_prefix(end);
			start = end;
			auto next = delim(source.substr(end + 1));
			if (next == std::string::npos)
				break;
			end = next + end + 1;
		}
		func(source.substr(start), true);
	}

	constexpr inline std::pair<std::string_view, std::string_view> single_split(std::string_view src, char delim) noexcept
	{
		size_t split_at = src.find_first_of(delim);
		if (split_at == std::string::npos)
			return {};
		return {src.substr(0, split_at), src.substr(split_at + 1) };
	}

	template <typename FUNC>
	requires std::is_invocable_v<FUNC, std::string_view, bool>
	inline void natural_split(std::string_view source, char delim, FUNC&& func) noexcept
	{
		size_t next = 0;
		while ((next = source.find_first_of(delim)) != std::string::npos)
		{
			func(source.substr(0, next), false);
			source.remove_prefix(next + 1);

			if ((next = source.find_first_not_of(delim)) == std::string::npos)
				return;

			source.remove_prefix(next);
		}

		if (!source.empty())
			func(source, true);
	}

	template <typename RESULT_TYPE = std::string_view, string_or_char DELIM>
	[[nodiscard]] constexpr inline std::vector<RESULT_TYPE> split(std::string_view source, DELIM&& delim) noexcept
	{
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::split(source, std::forward<DELIM>(delim), [&result](std::string_view str, bool last) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}

	template <typename RESULT_TYPE = std::string_view>
	[[nodiscard]] constexpr inline std::vector<RESULT_TYPE> split_on_any(std::string_view source, std::string_view delim) noexcept
	{
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::split_on_any(source, delim, [&result](std::string_view str, bool last) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}

	template <typename RESULT_TYPE = std::string_view, typename DELIM_FUNC>
	requires std::is_invocable_r_v<size_t, DELIM_FUNC, std::string_view>
	[[nodiscard]] std::vector<RESULT_TYPE> split_on(std::string_view source, DELIM_FUNC&& delim) noexcept(noexcept(delim(std::string_view{})))
	{
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::split_on(source, std::forward<DELIM_FUNC>(delim), [&result](std::string_view str, bool last) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}

	template <typename RESULT_TYPE = std::string_view, string_or_char DELIM>
	[[nodiscard]] inline std::vector<RESULT_TYPE> natural_split(std::string_view source, DELIM&& delim) noexcept
	{
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::natural_split(source, std::forward<DELIM>(delim), [&result](std::string_view str, bool last) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}

	template <std::ranges::range T, string_or_char DELIM>
	[[nodiscard]] inline auto join(T&& source, DELIM&& delim)
	{
		std::stringstream strm;
		bool first = true;
		for (auto&& p : std::forward<T>(source))
		{
			if (!first) strm << delim;
			strm << p;
			first = false;
		}
		return strm.str();
	}

	template <std::ranges::range... RANGES, string_or_char DELIM>
	[[nodiscard]] inline auto join_multiple(DELIM&& delim, RANGES&&... sources)
	{
		std::stringstream strm;
		bool first = true;
		([&]<typename RANGE>(RANGE&& source) {
			for (auto&& p : std::forward<RANGE>(source))
			{
				if (!first) strm << delim;
				strm << p;
				first = false;
			}
		}(std::forward<RANGES>(sources)), ...);
		return strm.str();
	}

	template <std::ranges::range T, string_or_char DELIM, string_or_char LAST_DELIM>
	[[nodiscard]] inline auto join_and(T&& source, DELIM&& delim, LAST_DELIM&& last_delim)
	{
		using std::begin;
		using std::end;
		using std::next;

		std::stringstream strm;
		bool first = true;
		
		auto&& endit = end(source);
		for (auto it = begin(source); it != endit; ++it)
		{
			if (!first)
			{
				if (next(it) == endit)
					strm << std::forward<LAST_DELIM>(last_delim);
				else
					strm << delim;
			}
			strm << *it;
			first = false;
		}
		return strm.str();
	}

	template <std::ranges::range T, typename FUNC, string_or_char DELIM>
	[[nodiscard]] inline auto join(T&& source, DELIM&& delim, FUNC&& transform_func)
	{
		std::stringstream strm;
		bool first = true;
		for (auto&& p : source)
		{
			if (!first) strm << delim;
			strm << transform_func(p);
			first = false;
		}
		return strm.str();
	}

	template <string_or_char NEEDLE, string_or_char REPLACE>
	inline void replace(std::string& subject, NEEDLE&& search, REPLACE&& replace)
	{
		using std::empty;
		using std::size;

		const auto search_sv = make_sv(search);
		const auto replace_sv = make_sv(replace);

		if (search_sv.empty())
			return;

		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			subject.replace(pos, search_sv.size(), replace_sv);
			pos += replace_sv.size();
		}
	}

	template <string_or_char DELIMITER = char, string_or_char ESCAPE = char>
	inline void quote(std::string& subject, DELIMITER delimiter = '"', ESCAPE escape = '\\')
	{
		const char replace_delim[]{ escape, delimiter, '\0' };
		const char replace_escape[]{ escape, escape, '\0' };
		if constexpr (requires { delimiter != escape; })
		{
			if (delimiter != escape)
				::ghassanpl::string_ops::replace(subject, escape, replace_escape);
		}
		else
			::ghassanpl::string_ops::replace(subject, escape, replace_escape);
		::ghassanpl::string_ops::replace(subject, delimiter, replace_delim);
		subject.insert(subject.begin(), delimiter);
		subject += delimiter;
	}

	template <string_or_char DELIMITER = char, string_or_char ESCAPE = char>
	[[nodiscard]] inline std::string quoted(std::string subject, DELIMITER&& delimiter = '"', ESCAPE&& escape = '\\')
	{
		::ghassanpl::string_ops::quote(subject, std::forward<DELIMITER>(delimiter), std::forward<ESCAPE>(escape));
		return subject;
	}

	template <string_or_char DELIMITER = char, string_or_char ESCAPE = char>
	[[nodiscard]] inline std::string quoted(std::string_view subject, DELIMITER&& delimiter = '"', ESCAPE&& escape = '\\')
	{
		auto result = std::string{ subject };
		::ghassanpl::string_ops::quote(result, std::forward<DELIMITER>(delimiter), std::forward<ESCAPE>(escape));
		return result;
	}

	template <string_or_char DELIMITER = char, string_or_char ESCAPE = char>
	[[nodiscard]] inline std::string quoted(const char* subject, DELIMITER&& delimiter = '"', ESCAPE&& escape = '\\')
	{
		return ::ghassanpl::string_ops::quoted(std::string{ subject }, std::forward<DELIMITER>(delimiter), std::forward<ESCAPE>(escape));
	}

	template <typename ESCAPE_FUNC>
	requires std::is_invocable_v<ESCAPE_FUNC, std::string_view> && std::is_constructible_v<std::string_view, std::invoke_result_t<ESCAPE_FUNC, std::string_view>>
	inline void escape(std::string& subject, std::string_view chars_to_escape, ESCAPE_FUNC&& escape_func)
	{
		if (chars_to_escape.empty())
			return;

		size_t pos = 0;
		while ((pos = subject.find_first_of(chars_to_escape, pos)) != std::string::npos)
		{
			auto escape_str = escape_func(subject.substr(pos, 1));
			subject.replace(pos, 1, escape_str);
			pos += escape_str.size();
		}
	}

	template <string_or_char ESCAPE = char>
	inline void escape(std::string& subject, std::string_view chars_to_escape, ESCAPE&& escape)
	{
		if (chars_to_escape.empty())
			return;

		auto escape_str = make_sv(escape);

		size_t pos = 0;
		while ((pos = subject.find_first_of(chars_to_escape, pos)) != std::string::npos)
		{
			subject.insert(pos, escape_str);
			pos += escape_str.size() + 1;
		}
	}

	template <typename STR, string_or_char ESCAPE = char>
	[[nodiscard]] inline std::string escaped(STR&& subject, std::string_view to_escape = "\"\\", ESCAPE&& escape_str = '\\')
	{
		auto result = std::string{ subject };
		::ghassanpl::string_ops::escape(result, to_escape, std::forward<ESCAPE>(escape_str));
		return result;
	}

	[[nodiscard]] inline std::string url_encode(std::string_view text)
	{
		std::string result;
		for (auto c : text)
		{
			if (ascii::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
			{
				result += c;
				continue;
			}

			result += '%';
			std::format_to(std::back_inserter(result), "{:02X}", (int)(unsigned char)c);
		}
		return result;
	}

	[[nodiscard]] inline std::string url_unencode(std::string_view text)
	{
		std::string result;
		while (!text.empty())
		{
			if (text[0] == '%')
			{
				text.remove_prefix(1);
				if (text.size() < 2)
					continue;
				uint8_t val{};
				std::from_chars(text.data(), text.data() + 2, val, 16);
				result += (char)val;
				text.remove_prefix(2);
				continue;
			}

			result += text[0];
			text.remove_prefix(1);
		}
		return result;
	}

#if defined(__cpp_lib_format)
	template <typename CHECKER>
	requires std::predicate<CHECKER, std::string_view>
	[[nodiscard]] inline std::string unique_name(std::string_view base_name, CHECKER&& checker)
	{
		std::string new_name = std::string{ base_name };
		size_t index = 1;
		while (!checker(std::string_view{ new_name }))
			new_name = std::format("{}{}", base_name, index++);
		return new_name;
	}
#endif

#if !__cpp_lib_to_chars

#else
	template <typename T>
	[[nodiscard]] inline auto from_chars(std::string_view str, T& value, const int base = 10) noexcept {
		return std::from_chars(str.data(), str.data() + str.size(), value, base);
	}
#endif

	/// TODO: Make this an actual range
	template <bool SINGLE>
	struct split_range
	{
		split_range(std::string_view source, char split_on) : mSource(source), mSplit(split_on) {}

		struct split_range_iterator
		{
			const char* RangeStart;
			const char* RangeEnd;
			const char* SourceEnd;
			char SplitChar;

			bool operator!=(const split_range_iterator& other) const { return RangeStart != other.SourceEnd; }
			split_range_iterator& operator++(int) {
				auto copy = *this;
				++* this;
				return copy;
			}

			split_range_iterator operator++() {
				auto rs = RangeEnd;
				auto se = SourceEnd;
				auto sc = SplitChar;

				if (SINGLE)
				{
					if (rs < se && *rs == sc)
						++rs;
				}
				else
				{
					while (rs < se && *rs == sc)
						++rs;
				}

				RangeStart = rs;

				while (rs < se && *rs != sc)
					++rs;

				RangeEnd = rs;
				return *this;
			}

			std::pair<const char*, const char*> operator*() const { return { RangeStart, RangeEnd }; }
		};

		split_range_iterator begin() {
			split_range_iterator it = { nullptr, mSource.data(), mSource.data() + mSource.size(), mSplit };
			++it;
			return it;
		}

		split_range_iterator end() {
			auto se = mSource.data() + mSource.size();
			return { se, se, se, mSplit };
		}

		std::string_view mSource;
		char mSplit;
	};

	/// static_assert(std::ranges::range<split_range<true>>);

	template <typename T, typename FUNC>
	requires std::is_arithmetic_v<T> && std::is_invocable_r_v<T, FUNC, std::string_view>
	std::vector<std::string_view> word_wrap(std::string_view _source, T max_width, FUNC width_getter)
	{
		std::vector<std::string_view> result;

		auto space_left = max_width;
		auto space_width = width_getter(" ");

		for (auto line : split_range<true>(_source, '\n'))
		{
			auto line_start = line.first;

			for (auto r : split_range<false>({ line.first, size_t(line.second - line.first) }, ' '))
			{
				const auto word_width = width_getter({ r.first, size_t(r.second - r.first) });
				const auto width = word_width + space_width;
				if (width > space_left)
				{
					result.emplace_back(line_start, size_t(r.first - line_start) - 1);
					space_left = max_width - word_width;
					line_start = r.first;
				}
				else
				{
					space_left -= width;
				}
			}

			result.emplace_back(line_start, size_t(line.second - line_start));
		}

		return result;
	}

	template <typename T>
	requires std::is_arithmetic_v<T>
	std::vector<std::string_view> word_wrap(std::string_view _source, T max_width, T letter_width)
	{
		return ::ghassanpl::string_ops::word_wrap(_source, max_width, [letter_width](std::string_view str) { return T(str.size() * letter_width); });
	}
}

#define GHPL_FORMAT_TEMPLATE typename... GHPL_ARGS
#define GHPL_FORMAT_ARGS std::string_view ghpl_fmt, GHPL_ARGS&&... ghpl_args
#define GHPL_FORMAT_FORWARD ghpl_fmt, std::forward<GHPL_ARGS>(ghpl_args)...
#define GHPL_FORMAT_CALL std::vformat(ghpl_fmt, std::make_format_args(std::forward<GHPL_ARGS>(ghpl_args)...))