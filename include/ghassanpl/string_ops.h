/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <vector>
#include <sstream>
#include <charconv>
#include <optional>
#include <ranges>
#include <numeric>

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

#include "expected.h"

static_assert(CHAR_BIT >= 8);

namespace ghassanpl::string_ops
{
	/// \defgroup StringOps String Operations
	/// Adds a few utility functions that deal with strings and string_views.
	/// @{
	
	/// The type is a stringable or a character
	/// \tparam CHAR_TYPE the base char type, char by default
	template <typename T, typename CHAR_TYPE = char>
	concept string_or_char = std::is_constructible_v<std::basic_string_view<CHAR_TYPE>, T> || std::is_constructible_v<CHAR_TYPE, T>;

	/// The type is "stringable", that is, a continuous range of characters
	/// \tparam CHAR_TYPE the character type, char by default
	template <typename T, typename CHAR_TYPE = char>
	concept stringable = (std::ranges::contiguous_range<T> && std::is_convertible_v<std::ranges::range_value_t<T>, CHAR_TYPE>);

	/// The type is a string with an 8-bit char type
	template <typename T>
	concept string8 = std::same_as<T, std::string> || std::same_as<T, std::u8string>;
	/// The type is convertible to a string view with an 8-bit char type
	template <typename T>
	concept stringable8 = std::convertible_to<T, std::string_view> || std::convertible_to<T, std::u8string_view>;
	/// The type is a string view with an 8-bit char type
	template <typename T>
	concept string_view8 = std::same_as<T, std::string_view> || std::same_as<T, std::u8string_view>;

	/// The type is a string with a 16-bit char type
	template <typename T>
	concept string16 = (sizeof(wchar_t) == sizeof(char16_t) && std::same_as<T, std::wstring>) || std::same_as<T, std::u16string>;
	/// The type is convertible to a string view with a 16-bit char type
	template <typename T>
	concept stringable16 = (sizeof(wchar_t) == sizeof(char16_t) && std::convertible_to<T, std::wstring_view>) || std::convertible_to<T, std::u16string_view>;
	/// The type is a string view with a 16-bit char type
	template <typename T>
	concept string_view16 = (sizeof(wchar_t) == sizeof(char16_t) && std::same_as<T, std::wstring_view>) || std::same_as<T, std::u16string_view>;

	/// The type is a string with an 32-bit char type
	template <typename T>
	concept string32 = (sizeof(wchar_t) == sizeof(char32_t) && std::same_as<T, std::wstring>) || std::same_as<T, std::u32string>;
	/// The type is convertible to a string view with a 32-bit char type
	template <typename T>
	concept stringable32 = (sizeof(wchar_t) == sizeof(char32_t) && std::convertible_to<T, std::wstring_view>) || std::convertible_to<T, std::u32string_view>;
	/// The type is a string view with a 32-bit char type
	template <typename T>
	concept string_view32 = (sizeof(wchar_t) == sizeof(char32_t) && std::same_as<T, std::wstring_view>) || std::same_as<T, std::u32string_view>;

	/// The default 16-bit char type for the current platform (wchar_t if it is 16-bit, char16_t otherwise)
	using wide_char16_t = std::conditional_t<sizeof(wchar_t) == sizeof(char16_t), wchar_t, char16_t>;

	/// The default 32-bit char type for the current platform (wchar_t if it is 32-bit, char32_t otherwise)
	using wide_char32_t = std::conditional_t<sizeof(wchar_t) == sizeof(char32_t), wchar_t, char32_t>;

	template <typename A, typename B>
	concept same_size_and_alignment = sizeof(A) == sizeof(B) && alignof(A) == alignof(B);

	/// Can a type be bit-cast to a native/utf char type?
	template <typename T>
	concept charable = (std::is_trivially_copyable_v<T> && ( /// Specifying all char types because they can have different sizes than implied
		same_size_and_alignment<T, wchar_t> ||
		same_size_and_alignment<T, char> ||
		same_size_and_alignment<T, char8_t> ||
		same_size_and_alignment<T, char16_t> ||
		same_size_and_alignment<T, char32_t>
	));

	/// Whether the type is a native char type
	template <typename T>
	concept char_type = std::same_as<T, char> || std::same_as<T, wchar_t>;

	/// Whether the type is a utf char type
	template <typename T>
	concept utf_type = std::same_as<T, char8_t> || std::same_as<T, char16_t> || std::same_as<T, char32_t>;

	/// Whether the type is a native or utf char type
	template <typename T>
	concept stringable_base_type = char_type<T> || utf_type<T>;

	/// The utf char type corresponding to the charable type
	template <charable T>
	using charable_utf_t = std::conditional_t<same_size_and_alignment<T, char8_t>, char8_t, 
		std::conditional_t<same_size_and_alignment<T, char16_t>, char16_t, 
		std::conditional_t<same_size_and_alignment<T, char32_t>, char32_t, 
		void
	>>>;

	/// The native char type corresponding to the charable type
	template <charable T>
	using charable_char_t = std::conditional_t<same_size_and_alignment<T, char>, char,
		std::conditional_t<same_size_and_alignment<T, wide_char16_t>, wide_char16_t,
		std::conditional_t<same_size_and_alignment<T, wide_char32_t>, wide_char32_t,
		void
	>>>;

	template <charable T>
	using best_stringable_type = std::conditional_t<stringable_base_type<T>,
		T,
		charable_char_t<T>
	>;

	/// \name Make Functions
	/// Functions that create `string_view` and `string` types from various values
	/// 
	/// \par Rationale
	/// Even though C++20 has a range constructor, it uses `operator-` on its arguments, which means that iterators from disparate string_views do not work, even
	/// if they point to a contiguous string range (for example, were made from substrings of the same string_view). Hence these functions.
	/// They should work with any pair of iterators (no support for sentinels yet, unfortunately), support nulls, and are almost as strong as the string_view range constructor
	/// in terms of exception and type safety. As with the respective constructors, undefined behavior when `start` > `end`.
	/// 
	/// @{

	template <typename C = char>
	[[nodiscard]] constexpr std::basic_string_view<C> make_sv(std::nullptr_t, std::nullptr_t) noexcept { return {}; }

	template <stringable_base_type CT, std::contiguous_iterator IT, std::contiguous_iterator IT2>
	requires charable<std::iter_value_t<IT>>
	[[nodiscard]] constexpr auto make_sv(IT start, IT2 end) noexcept(noexcept(std::to_address(start))) { 
		return std::basic_string_view<CT>{
			reinterpret_cast<CT const*>(std::to_address(start)),
			static_cast<size_t>(std::to_address(end) - std::to_address(start)) 
		};
	}

	template <std::contiguous_iterator IT, std::contiguous_iterator IT2>
	requires charable<std::iter_value_t<IT>>
	[[nodiscard]] constexpr auto make_sv(IT start, IT2 end) noexcept(noexcept(std::to_address(start))) { 
		using char_type = best_stringable_type<std::iter_value_t<IT>>;
		return make_sv<char_type, IT, IT2>(std::move(start), std::move(end));
	}

	template <typename T>
	requires stringable_base_type<std::remove_cvref_t<T>>
	[[nodiscard]] constexpr auto make_sv(T&& single_char) noexcept { 
		static_assert(!std::is_rvalue_reference_v<decltype(single_char)>, "cannot create string_view's from single char rvalues");
		return make_sv(&single_char, &single_char + 1); 
	}

	template <charable T>
	[[nodiscard]] constexpr auto make_sv(const T* str) noexcept { 
		using CT = best_stringable_type<T>;
		return str ? std::basic_string_view<CT>{ reinterpret_cast<const CT*>(str) } : std::basic_string_view<CT>{};
	}

	template <typename C>
	[[nodiscard]] constexpr std::basic_string_view<C> make_sv(std::basic_string_view<C> id) noexcept { return id; }
	template <typename C>
	[[nodiscard]] constexpr std::basic_string_view<C> make_sv(std::basic_string<C> const& id) noexcept { return id; }
	template <typename C>
	[[nodiscard]] constexpr std::basic_string_view<C> make_sv(std::basic_string<C>&& id) noexcept = delete;

	template <std::ranges::range RANGE>
	requires charable<std::ranges::range_value_t<RANGE>>
	[[nodiscard]] constexpr auto make_sv(RANGE&& range) noexcept
	{
		return make_sv(std::ranges::begin(range), std::ranges::end(range));
	}

	template <typename... NONARGS, typename... ARGS>
	[[nodiscard]] constexpr auto make_string(ARGS&&... args) { 
		auto sv = make_sv<NONARGS...>(std::forward<ARGS>(args)...);
		using char_type = typename decltype(sv)::value_type;
		return std::basic_string<char_type>{ sv };
	}
	
	/*
	template <std::contiguous_iterator IT, std::contiguous_iterator IT2>
	requires std::is_same_v<std::iter_value_t<IT>, char>&& std::is_same_v<std::iter_value_t<IT2>, char>
	[[nodiscard]] inline std::string make_string(IT start, IT2 end) noexcept(noexcept(std::to_address(start))) { return std::string{ ::ghassanpl::string_ops::make_sv(start, end) }; }
	*/

	/// @}
	
	/// Casts a string_view to a string_view with a different char type via a simple reinterpret_cast
	template <typename COUT, typename CIN>
	requires charable<COUT> && charable<CIN> && same_size_and_alignment<COUT, CIN>
	[[nodiscard]] constexpr std::basic_string_view<COUT> string_view_cast(std::basic_string_view<CIN> id) noexcept
	{
		return { reinterpret_cast<COUT const*>(id.data()), id.size() };
	}

	/// \name to_string Functions
	/// Basic identity and utility `to_string` functions.
	/// \see Stringification
	/// @{

	[[nodiscard]] inline std::string to_string(std::string_view from) noexcept { return std::string{ from }; }

	[[nodiscard]] inline std::string to_string(std::u8string_view from) noexcept { return std::string{ from.data(), from.data() + from.size() }; }

	template<typename T>
	[[nodiscard]] inline std::string to_string(T const& t) requires requires { std::to_string(t); } { return std::to_string(t); }

	[[nodiscard]] constexpr std::string const& to_string(std::same_as<std::string> auto const& s) { return s; }

	template<typename T>
	[[nodiscard]] inline std::string to_string(std::optional<T> const& o) { if (o.has_value()) return std::to_string(o.value()); return "(empty)"; }

	/// @}

	/// Creates a `string_view` with its beginning moved back by `n` characters, limited to a parent range.
	/// \par Example
	/// ```
	/// parent = "HelloWorld"; 
	/// child = parent.substr(5); 
	/// n = 1; 
	/// back(child, parent, n) => "oWorld";
	/// ```
	/// 
	/// \param child_to_back_up the input string_view
	/// \param parent a string_view that `child_to_back_up` is a subview of
	/// \param n how many characters to back up
	[[nodiscard]] constexpr std::string_view back(std::string_view child_to_back_up, std::string_view parent, size_t n = 1) noexcept
	{
		return make_sv(std::max(child_to_back_up.data() - n, parent.data()), child_to_back_up.data() + child_to_back_up.size());
	}

	/// Creates a `string_view` with its beginning moved back by `n` characters.
	/// \par Example
	/// ```
	/// parent = "HelloWorld"; 
	/// child = parent.substr(5); 
	/// n = 1; 
	/// back(child, n) => "oWorld";
	/// ```
	/// 
	/// \par Note
	/// This is, of course, unsafe to do if the new beginning is beyond the underlying string's range
	/// 
	/// \param child_to_back_up the input string_view
	/// \param n how many characters to back up
	[[nodiscard]] constexpr std::string_view back(std::string_view child_to_back_up, size_t n = 1) noexcept
	{
		return make_sv(child_to_back_up.data() - n, child_to_back_up.data() + child_to_back_up.size());
	}

	/// Checks if `smaller_string` is a true subset of `big_string` (true subset meaning they view over overlapping memory subregions)
	[[nodiscard]] constexpr bool is_inside(std::string_view big_string, std::string_view smaller_string)
	{
		return big_string.data() - smaller_string.data() >= 0 && (smaller_string.data() + smaller_string.size()) - (big_string.data() + big_string.size()) >= 0;
	}

	namespace ascii
	{

		/// \defgroup ASCII ASCII
		/// Functions that operate on codepoints and strings encoded as ASCII.
		/// 
		/// \ingroup StringOps
		/// @{

		/// \internal
		/// We are using numbers (e.g. 65) instead of character literals (e.g. 'A'), because the encoding of this source file might not be ASCII-based
		/// Thanks to \@fmatthew5876 for inspiration
		/// \endinternal

#if 0
		constexpr uint64_t is_flag_set(std::array<uint64_t, 2> flags, char32_t cp) noexcept
		{
			const auto flag_el = (cp >> 6) & 1;
			const auto bit = uint64_t(1) << (cp & 63);
			return (flags[flag_el] & bit) != 0;
		}

		template <uint64_t HIGH, uint64_t LOW>
		constexpr bool is_flag_set(char32_t cp) noexcept
		{
			const auto low_mask = uint64_t(0) - (cp >> 6 == 0);
			const auto high_mask = uint64_t(0) - (cp >> 6 == 1);
			const auto bit = uint64_t(1) << (cp & 63);

			return ((bit & low_mask) & LOW) or ((bit & high_mask) & HIGH);
		}

		constexpr std::array<uint64_t, 2> get_flags_for(std::string_view str) noexcept
		{
			std::array<uint64_t, 2> result{ 0,0 };
			for (auto c : str)
				result[(char32_t(c) >> 6) & 1] |= (char32_t(c) & 63);
			return result;
		}

		template <typename FUNC>
		constexpr std::array<uint64_t, 2> get_flags_for(FUNC&& pred) noexcept
		{
			std::array<uint64_t, 2> result{ 0,0 };
			for (char32_t c = 0; c < 128; ++c)
				if (pred(c))
					result[(char32_t(c) >> 6) & 1] |= (char32_t(c) & 63);
			return result;
		}
#endif

		/// \name is* and to* functions
		/// These are our own versions of \<cctype\> functions that do not block, are defined (false) for values outside of uint8_t, and do not depend on locale (plus you can take pointers to them).
		/// @{

		[[nodiscard]] constexpr bool isalpha(char32_t cp) noexcept { return (cp >= 65 && cp <= 90) || (cp >= 97 && cp <= 122); }
		[[nodiscard]] constexpr bool isdigit(char32_t cp) noexcept { return cp >= 48 && cp <= 57; }
		[[nodiscard]] constexpr bool isodigit(char32_t cp) noexcept { return cp >= 48 && cp <= 55; }
		[[nodiscard]] constexpr bool isxdigit(char32_t d) noexcept { return (d >= 48 && d <= 57) || (d >= 65 && d <= 70) || (d >= 97 && d <= 102); }
		[[nodiscard]] constexpr bool isalnum(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isdigit(cp) || ::ghassanpl::string_ops::ascii::isalpha(cp); }
		[[nodiscard]] constexpr bool isident(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isdigit(cp) || ::ghassanpl::string_ops::ascii::isalpha(cp) || cp == 95; }
		[[nodiscard]] constexpr bool isidentstart(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isalpha(cp) || cp == 95; }
		[[nodiscard]] constexpr bool isspace(char32_t cp) noexcept { return (cp >= 9 && cp <= 13) || cp == 32; }
		[[nodiscard]] constexpr bool ispunct(char32_t cp) noexcept { return (cp >= 33 && cp <= 47) || (cp >= 58 && cp <= 64) || (cp >= 91 && cp <= 96) || (cp >= 123 && cp <= 126); }
		[[nodiscard]] constexpr bool islower(char32_t cp) noexcept { return cp >= 97 && cp <= 122; }
		[[nodiscard]] constexpr bool isupper(char32_t cp) noexcept { return cp >= 65 && cp <= 90; }
		[[nodiscard]] constexpr bool iscntrl(char32_t cp) noexcept { return cp == 0x7F || cp < 0x20; }
		[[nodiscard]] constexpr bool isblank(char32_t cp) noexcept { return cp == 32 || cp == 9; }
		[[nodiscard]] constexpr bool isgraph(char32_t cp) noexcept { return cp >= 33 && cp <= 126; }
		[[nodiscard]] constexpr bool isprint(char32_t cp) noexcept { return cp >= 32 && cp <= 126; }

		[[nodiscard]] constexpr char32_t toupper(char32_t cp) noexcept {
			return (cp >= 97 && cp <= 122) ? (cp ^ 0b100000) : cp;
		}
		[[nodiscard]] constexpr char32_t tolower(char32_t cp) noexcept {
			return (cp >= 65 && cp <= 90) ? (cp | 0b100000) : cp;
		}

		/// @}

		namespace detail
		{
			template <auto FUNC>
			consteval size_t count_chars() { 
				const auto v = std::views::iota(char32_t{ 1 }, char32_t{ 127 });
				return std::ranges::count_if(v, FUNC);
			}
			template <typename T, auto FUNC>
			constexpr auto compute_characters_matching() {
				constexpr size_t char_count = count_chars<FUNC>();
				std::array<T, char_count> result{};
				std::ranges::copy_if(std::views::iota(T{ 1 }, T{ 127 }), result.begin(), FUNC);
				return result;
			}
		}

		/// \name *_chars variables
		/// Constexpr arrays listing the characters that match their respective ascii::is* functions
		/// @{
		constexpr inline auto alphabetic_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isalpha>(); ///< All characters that match \c ascii::isalpha
		constexpr inline auto digit_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isdigit>(); ///< All characters that match \c ascii::isdigit
		constexpr inline auto octal_digit_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isodigit>(); ///< All characters that match \c ascii::isodigit
		constexpr inline auto hex_digit_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isxdigit>(); ///< All characters that match \c ascii::isxdigit
		constexpr inline auto alphanumeric_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isalnum>(); ///< All characters that match \c ascii::isalnum
		constexpr inline auto identifier_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isident>(); ///< All characters that match \c ascii::isident
		constexpr inline auto identifier_start_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isidentstart>(); ///< All characters that match \c ascii::isidentstart
		constexpr inline auto whitespace_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isspace>(); ///< All characters that match \c ascii::isspace
		constexpr inline auto punctuation_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::ispunct>(); ///< All characters that match \c ascii::ispunct
		constexpr inline auto lowercase_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::islower>(); ///< All characters that match \c ascii::islower
		constexpr inline auto uppercase_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isupper>(); ///< All characters that match \c ascii::isupper
		constexpr inline auto control_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::iscntrl>(); ///< All characters that match \c ascii::iscntrl
		constexpr inline auto blank_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isblank>(); ///< All characters that match \c ascii::isblank
		constexpr inline auto graphical_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isgraph>(); ///< All characters that match \c ascii::isgraph
		constexpr inline auto printable_chars = detail::compute_characters_matching<char, ::ghassanpl::string_ops::ascii::isprint>(); ///< All characters that match \c ascii::isprint
		/// @}

		/// Returns true if the given `str` is a C-style identifier (e.g. matches `/[\w_][\w_0-9]+/`)
		template <stringable T>
		[[nodiscard]] constexpr bool is_identifier(T const& str) noexcept {
			if (std::ranges::empty(str)) return false;
			return ::ghassanpl::string_ops::ascii::isidentstart(*std::ranges::begin(str)) && std::ranges::all_of(std::views::drop(str, 1), ::ghassanpl::string_ops::ascii::isident);
		}

		/// Returns a copy of the string with all characters transformed to lower case
		template <stringable T>
		[[nodiscard]] constexpr std::string tolower(T const& str) noexcept {
			using std::ranges::begin;
			using std::ranges::end;
			std::string result;
			if constexpr (std::ranges::sized_range<T>)
				result.reserve(std::ranges::size(str));
			std::transform(begin(str), end(str), std::back_inserter(result), [](char cp) { return (char)::ghassanpl::string_ops::ascii::tolower(cp); });
			return result;
		}

		/// \copydoc tolower(T const& str)
		[[nodiscard]] constexpr std::string tolower(std::string str) noexcept {
			std::ranges::transform(str, std::ranges::begin(str), [](char cp) { return (char)::ghassanpl::string_ops::ascii::tolower(cp); });
			return str;
		}

		/// \copydoc tolower(T const& str)
		[[nodiscard]] constexpr std::string tolower(const char* str) noexcept {
			if (str)
				return tolower(std::string{ str });
			return {};
		}

		/// Returns a copy of the string with all characters transformed to upper case
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

		/// \copydoc toupper(T const& str)
		[[nodiscard]] inline std::string toupper(std::string str) noexcept {
			std::ranges::transform(str, std::ranges::begin(str), [](char cp) { return (char)::ghassanpl::string_ops::ascii::toupper(cp); });
			return str;
		}

		/// \copydoc toupper(T const& str)
		[[nodiscard]] inline std::string toupper(const char* str) noexcept {
			if (str)
				return toupper(std::string{ str });
			return {};
		}

		/// Returns a copy of the string with the first letter transformed to upper case if possible
		[[nodiscard]] inline std::string capitalize(std::string str) noexcept {
			if (str.empty()) return str;
			str[0] = (char)::ghassanpl::string_ops::ascii::toupper(str[0]);
			return str;
		}

		/// Convert a number between 0 and 9 to its ASCII representation (only gives meaningful results with arguments between 0 and 9)
		[[nodiscard]] constexpr char32_t number_to_digit(int v) noexcept { return char32_t(v) + 48; }
		/// Convert a number between 0 and 15 to its ASCII representation (only gives meaningful results with arguments between 0 and 15)
		[[nodiscard]] constexpr char32_t number_to_xdigit(int v) noexcept { return (v > 9) ? (char32_t(v - 10) + 65) : (char32_t(v) + 48); }

		/// Convert an ASCII digit character to its numerical value (only gives meaningful results with valid digit arguments)
		[[nodiscard]] constexpr int digit_to_number(char32_t cp) noexcept { return int(cp - 48); }
		/// Convert an ASCII xdigit to its numerical value (only gives meaningful results with valid xdigit arguments)
		//[[nodiscard]] constexpr int xdigit_to_number(char32_t cp) noexcept { return (cp >= 97 && cp <= 102) ? int(cp - 97) : int((cp >= 65 && cp <= 70) ? (cp - 55) : (cp - 48)); }
		[[nodiscard]] constexpr int xdigit_to_number(char32_t cp) noexcept { return isdigit(cp) ? int(cp - 48) : ((int(cp) & ~0b100000) - 55); }

		/// \name Case-invariant Comparisons
		/// TODO: Add rfind, find_first_of, find_last_of, find_first_not_of, find_last_not_of
		/// @{
		[[nodiscard]] constexpr bool strings_equal_ignore_case(std::string_view sa, std::string_view sb)
		{
			return std::ranges::equal(sa, sb, [](char a, char b) { return ::ghassanpl::string_ops::ascii::toupper(a) == ::ghassanpl::string_ops::ascii::toupper(b); });
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

		[[nodiscard]] constexpr auto string_find_ignore_case(std::string_view haystack, std::string_view pattern)
		{
			return std::ranges::search(
				haystack,
				pattern,
				[](char ch1, char ch2) { return ::ghassanpl::string_ops::ascii::tolower(ch1) == ::ghassanpl::string_ops::ascii::tolower(ch2); }
			).begin();
		}

		[[nodiscard]] constexpr auto string_find_last_ignore_case(std::string_view a, std::string_view b)
		{
			return std::ranges::find_end(
				a,
				b,
				[](char ch1, char ch2) { return ::ghassanpl::string_ops::ascii::tolower(ch1) == ::ghassanpl::string_ops::ascii::tolower(ch2); }
			).begin();
		}

		[[nodiscard]] constexpr auto string_contains_ignore_case(std::string_view a, std::string_view b)
		{
			return string_find_ignore_case(a, b) != a.end();
		}

		[[nodiscard]] constexpr bool lexicographical_compare_ignore_case(std::string_view first, std::string_view second)
		{
			return std::ranges::lexicographical_compare(first, second,
				[](char a, char b) { return ::ghassanpl::string_ops::ascii::toupper(a) < ::ghassanpl::string_ops::ascii::toupper(b); });
		}

		[[nodiscard]] constexpr auto lexicographical_compare_ignore_case_three_way(std::string_view a, std::string_view b)
		{
			return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(),
				[](char ca, char cb) { return ::ghassanpl::string_ops::ascii::toupper(ca) <=> ::ghassanpl::string_ops::ascii::toupper(cb); });
		}

		[[nodiscard]] constexpr uint64_t hash_ignore_case(std::string_view str)
		{
			/// FNV-1a hash
			uint64_t result = 0xcbf29ce484222325;
			for (auto byte : str)
				result = (result ^ static_cast<uint8_t>(tolower(byte))) * 0x00000100000001b3U;
			return result;
		}

		namespace detail
		{
			struct string_view_case_insensitive
			{
				std::string_view value;

				constexpr bool operator ==(std::string_view a) const noexcept { return strings_equal_ignore_case(value, a); }
				constexpr auto operator <=>(std::string_view a) const noexcept { return lexicographical_compare_ignore_case_three_way(value, a); }

				constexpr bool operator ==(string_view_case_insensitive const& other) const noexcept { return strings_equal_ignore_case(value, other.value); }
				constexpr auto operator <=>(string_view_case_insensitive const& other) const noexcept { return lexicographical_compare_ignore_case_three_way(value, other.value); }
			};
		}

		/// A version of the `sv` suffix that returns a special type allowing for case-insensitive comparisons (e.g. `if (str == "hello"_i)`)
		[[nodiscard]] consteval detail::string_view_case_insensitive operator"" _i(const char* str, size_t size) noexcept { return detail::string_view_case_insensitive{ std::string_view{str, str + size} }; }

		/// @}

		/// @}
	}

#pragma push_macro("isascii")
#undef isascii
	/// Returns true if `cp` is an ascii codepoint
	[[nodiscard]] constexpr bool isascii(char32_t cp) noexcept { return cp <= 127; }
#pragma pop_macro("isascii")

	/// Returns true if `cp` is an ascii codepoint
	[[nodiscard]] constexpr bool is_ascii(char32_t cp) noexcept { return cp <= 127; }

	/// A pre-C++23 version of `str.contains(c)`
	[[nodiscard]] constexpr bool string_contains(std::string_view str, char c)
	{
#if __cpp_lib_string_contains
		return str.contains(c);
#else
		return str.find(c) != std::string_view::npos;
#endif
	}

	[[nodiscard]] constexpr auto data_end(std::string_view str) noexcept { return str.data() + str.size(); }

	/// Gets a substring of `str` starting at `start` and containing `count` characters. The result will always be valid, clamped to the bounds of `str` (or empty).
	/// \param count the (maximum) number of characters to get
	/// \param start if negative, starts `-start` characters before the end
	[[nodiscard]] inline std::string_view substr(std::string_view str, intptr_t start, size_t count = std::string::npos) noexcept
	{
		if (start < 0)
		{
			start = str.size() + start;
			if (start < 0) start = 0;
		}
		if (start >= static_cast<intptr_t>(str.size())) return {};
		return str.substr(start, static_cast<size_t>(count));
	}

	/// Returns a substring containing the `count` leftmost characters of `str`. Always valid, clamped to the bounds of `str` (or empty).
	[[nodiscard]] inline std::string_view prefix(std::string_view str, size_t count) noexcept {
		return str.substr(0, count);
	}

	/// Returns a substring created by removing `count` characters from the end. Always valid, clamped to the bounds of `str` (or empty).
	[[nodiscard]] inline std::string_view without_suffix(std::string_view str, size_t count) noexcept {
		return str.substr(0, str.size() - std::min(str.size(), count));
	}

	/// Returns a substring containing the `count` rightmost characters of `str`. Always valid, clamped to the bounds of `str` (or empty).
	[[nodiscard]] inline std::string_view suffix(std::string_view str, size_t count) noexcept { 
		return str.substr(str.size() - std::min(count, str.size()));
	}

	/// Returns a substring created by removing `count` characters from the start. Always valid, clamped to the bounds of `str` (or empty).
	[[nodiscard]] inline std::string_view without_prefix(std::string_view str, size_t count) noexcept {
		return str.substr(std::min(str.size(), count));
	}

	/// Erases all characters in `str` outside of the range `[start, start + count]`. Always safe.
	inline void erase_outside_n(std::string& str, size_t start, size_t count) noexcept {
		str.erase(std::min(start + count, str.size()));
		str.erase(0, std::min(start, str.size()));
	}

	/// Erases all characters in `str` outside of the range `[from, to]`. Always safe.
	/// If from > to, acts as if calling `erase_outside_from_to(str, to, from)`
	inline void erase_outside_from_to(std::string& str, size_t from, size_t to) noexcept {
		const auto from_ = std::min(from, to);
		const auto to_ = std::max(from, to);
		erase_outside_n(str, from_, to_ - from_);
	}

	/// \name Trimming Functions
	/// Functions that trim (remove ascii whitespace from) strings and string_views.
	/// @{

	[[nodiscard]] constexpr std::string_view trimmed_whitespace_right(std::string_view str) noexcept { return make_sv(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base()); }
	[[nodiscard]] constexpr std::string_view trimmed_whitespace_left(std::string_view str) noexcept { return make_sv(std::ranges::find_if_not(str, ::ghassanpl::string_ops::ascii::isspace), str.end()); }
	[[nodiscard]] constexpr std::string_view trimmed_whitespace(std::string_view str) noexcept { return trimmed_whitespace_left(trimmed_whitespace_right(str)); }
	[[nodiscard]] constexpr std::string_view trimmed_until(std::string_view str, char chr) noexcept { return make_sv(std::ranges::find(str, chr), str.end()); }
	[[nodiscard]] constexpr std::string_view trimmed(std::string_view str, char chr) noexcept { return make_sv(std::ranges::find_if_not(str, [chr](char c) { return c == chr; }), str.end()); }

	[[nodiscard]] constexpr std::string trimmed_whitespace_right(std::string str) noexcept { str.erase(std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base(), str.end()); return str; }
	[[nodiscard]] constexpr std::string trimmed_whitespace_left(std::string str) noexcept { str.erase(str.begin(), std::ranges::find_if_not(str, ::ghassanpl::string_ops::ascii::isspace)); return str; }
	[[nodiscard]] constexpr std::string trimmed_whitespace(std::string str) noexcept { return trimmed_whitespace_left(trimmed_whitespace_right(std::move(str))); }
	[[nodiscard]] constexpr std::string trimmed_until(std::string str, char chr) noexcept { str.erase(str.begin(), std::ranges::find(str, chr)); return str; }
	[[nodiscard]] constexpr std::string trimmed(std::string str, char chr) noexcept { str.erase(str.begin(), std::ranges::find_if_not(str, [chr](char c) { return c == chr; })); return str; }
	//[[nodiscard]] inline std::string_view trimmed(std::string_view str) noexcept { if (!str.empty()) str.remove_prefix(1); return str; }
	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	[[nodiscard]] inline std::string_view trimmed_while(std::string_view str, FUNC&& func) noexcept { return ::ghassanpl::string_ops::make_sv(std::find_if_not(str.begin(), str.end(), std::forward<FUNC>(func)), str.end()); }

	/// TODO: trim_*(std::string&) overloads

	constexpr void trim_whitespace_right(std::string_view& str) noexcept { str = make_sv(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base()); }
	constexpr void trim_whitespace_left(std::string_view& str) noexcept { str = make_sv(std::ranges::find_if_not(str, ::ghassanpl::string_ops::ascii::isspace), str.end()); }
	constexpr void trim_whitespace(std::string_view& str) noexcept { trim_whitespace_left(str); trim_whitespace_right(str); }
	constexpr void trim_until(std::string_view& str, char chr) noexcept { str = trimmed_until(str, chr); }
	constexpr void trim(std::string_view& str, char chr) noexcept { str = trimmed(str, chr); }
	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	constexpr void trim_while(std::string_view& str, FUNC&& func) noexcept { str = trimmed_while(str, std::forward<FUNC>(func)); }

	/// @}

	/// Checks if `cp` is any of the characters in `chars`
	template <std::ranges::random_access_range T>
	requires stringable_base_type<std::ranges::range_value_t<T>>
	[[nodiscard]] constexpr bool isany(char32_t cp, T&& chars) noexcept {
		using char_type = std::ranges::range_value_t<T>;
		return std::ranges::find(chars, static_cast<char_type>(cp)) != std::ranges::end(chars);
	}
	/// A \c isany overload that takes a single character
	constexpr bool isany(char32_t c, char32_t c2) noexcept { return c == c2; }

	/// \name Consume Functions
	/// Functions that "consume" parts of a `string_view` (that is, remove a section from the beginning or end if the conditions apply).
	/// Most of the functions return the consumed part, or 'true/false' if the part to be consumed is given explicitly.
	/// These functions do nothing (or the maximum safe amount) if there is nothing appropriate available to consume.
	/// @{

	/// Consumes and returns the first character in the str, or \0 if no more characters.
	[[nodiscard]] inline char consume(std::string_view& str)
	{
		if (str.empty())
			return {};
		const auto result = str[0];
		str.remove_prefix(1);
		return result;
	}

	/// Consumes the character `val` if it's at the beginning of `str`
	/// \returns whether it actually consumed
	[[nodiscard]] inline bool consume(std::string_view& str, char val)
	{
		if (str.starts_with(val))
		{
			str.remove_prefix(1);
			return true;
		}
		return false;
	}

	/// Consumes the string `val` if it's at the beginning of `str`. 
	/// \returns whether it actually consumed
	[[nodiscard]] inline bool consume(std::string_view& str, std::string_view val)
	{
		if (str.starts_with(val))
		{
			str.remove_prefix(val.size());
			return true;
		}
		return false;
	}

	/// Consumes any of the characters in 'chars' if it's the first char of `str`. 
	/// \returns the consumed character, or \0 if none found
	template <typename... ARGS>
	[[nodiscard]] inline char consume_any(std::string_view& str, ARGS&&... args)
	{
		if (!str.empty() && (isany(str[0], args) || ...))
		{
			const auto result = str[0];
			str.remove_prefix(1);
			return result;
		}
		return 0;
	}

	/// Consumes a character from the beginning of `str` if it matches `pred(str[0])`.
	/// \returns the matched character, or \0 if no match
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

	/// Consumes the first character from `str`, returning it, or `or_else` if string is empty.
	[[nodiscard]] inline char consume_or(std::string_view& str, char or_else)
	{
		if (str.empty())
			return or_else;
		const auto result = str[0];
		str.remove_prefix(1);
		return result;
	}

	/// Consumes the last character from `str` if it matches `val`.
	/// \returns whether it consumed
	/// \see consume(std::string_view&, char)
	[[nodiscard]] inline bool consume_at_end(std::string_view& str, char val)
	{
		if (str.ends_with(val))
		{
			str.remove_suffix(1);
			return true;
		}
		return false;
	}


	/// Consumes the string `val` from the end `str`
	/// \returns whether it consumed
	/// \see consume(std::string_view&, char)
	[[nodiscard]] inline bool consume_at_end(std::string_view& str, std::string_view val)
	{
		if (str.ends_with(val))
		{
			str.remove_suffix(val.size());
			return true;
		}
		return false;
	}

	/// Consumes characters from the beginning of `str` while they match `pred(str[0])`.
	/// \returns the consumed prefix as a string_view
	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	[[nodiscard]] inline std::string_view consume_while(std::string_view& str, FUNC&& pred)
	{
		const auto start = str.begin();
		while (!str.empty() && pred(str[0]))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	/// Consumes characters from the beginning of `str` while they are equal to `c`.
	/// \returns the consumed prefix as a string_view
	[[nodiscard]] inline std::string_view consume_while(std::string_view& str, char c)
	{
		const auto start = str.begin();
		while (str.starts_with(c))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	/// Consumes a run of any of the characters in 'chars' at the beginning of str
	/// \returns the consumed character, or \0 if none found
	template <typename... ARGS>
	[[nodiscard]] inline std::string_view consume_while_any(std::string_view& str, ARGS&&... args)
	{
		const auto start = str.begin();
		while (!str.empty() && (isany(str[0], args) || ...))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	/// Consumes characters from the beginning of `str` until one matches `pred(str[0])`, exclusive.
	/// \returns the consumed prefix as a string_view
	template <typename FUNC>
	requires std::is_invocable_r_v<bool, FUNC, char>
	[[nodiscard]] inline std::string_view consume_until(std::string_view& str, FUNC&& pred)
	{
		const auto start = str.begin();
		while (!str.empty() && !pred(str[0]))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	/// Consumes characters from the beginning of `str` until one is equal to `c`, exclusive.
	/// \returns the consumed prefix as a string_view
	[[nodiscard]] inline std::string_view consume_until(std::string_view& str, char c)
	{
		const auto start = str.begin();
		while (!str.empty() && str[0] != c)
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	/// Consumes characters from the beginning of `str` until the string starts with `end`, exclusive.
	/// \returns the consumed prefix as a string_view
	[[nodiscard]] inline std::string_view consume_until(std::string_view& str, std::string_view end)
	{
		auto it = std::ranges::search(str, end).begin();
		auto result = make_sv(str.begin(), it);
		str = { it, str.end() };
		return result;
	}

	/*
	/// Consumes characters from the beginning of `str` until one is equal to any in `chars`, exclusive.
	/// \returns the consumed prefix as a string_view
	template <typename T>
	requires std::constructible_from<std::string_view, T>
	[[nodiscard]] inline std::string_view consume_until_any(std::string_view& str, T&& chars)
	{
		const auto charssv = std::string_view{ chars };
		const auto start = str.begin();
		while (!str.empty() && !charssv.contains(str[0]))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}
	*/

	/// Consumes characters from the beginning of `str` until one is equal to any in the parameter pack, exclusive.
	/// \returns the consumed prefix as a string_view
	template <typename... ARGS>
	[[nodiscard]] inline std::string_view consume_until_any(std::string_view& str, ARGS&&... args)
	{
		const auto start = str.begin();
		while (!str.empty() && !(isany(str[0], args) || ...))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	/// Consumes characters from the beginning of `str` until one is equal to `c`, **inclusive**.
	/// \returns the consumed prefix as a string_view
	[[nodiscard]] inline std::string_view consume_until_delim(std::string_view& str, char c)
	{
		/// TODO: Should this return a sv including the delimiter?

		const auto start = str.begin();
		while (!str.empty() && str[0] != c)
			str.remove_prefix(1);
		std::ignore = consume(str, c);
		return make_sv(start, str.begin());
	}

	/// Consumes at most `n` characters from the beginning of `str`.
	/// \returns the consumed prefix as a string_view
	[[nodiscard]] inline std::string_view consume_n(std::string_view& str, size_t n)
	{
		n = std::min(str.size(), n);
		auto result = str.substr(0, n);
		str.remove_prefix(n);
		return result;
	}

	/// Consumes at most `n` characters from the beginning of `str` that match `pred(str[0])`.
	/// \returns the consumed prefix as a string_view
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

	/// Consumes a list of `delimiter`-delimited strings, calling `callback(str)` each time; whitespaces before and after items are trimmed.
	/// \attention `callback` will be called with the ENTIRE string left after consuming the delimiter, so
	/// you have to consume the list elements yourself!
	/// \par Example
	/// ```cpp
	/// consume_delimited_list_non_empty("alpha, beta, gamma, omega", ",", [](auto& sv) { println("'{}'", consume_until(sv, ',')); return true; });
	/// >> will print
	/// 'alpha'
	/// 'beta'
	/// 'gamma'
	/// 'omega'
	/// ```
	/// \param callback must be invocable as `callback(string_view&) -> bool`; if the callback returns false, consumption is stopped; otherwise, 
	///		this function must consume its item from the given string view
	/// \returns `true` if consumption ended, either by making the string empty, or encountering a non-delimiter after callback; `false` if callback returned false at any point
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

	/// Consumes a list of `delimiter`-delimited strings, ended with `closer`, calling `callback(str)` each time; whitespaces before and after items are trimmed.
	/// Stops when `closer` is consumed.
	/// \attention `callback` will be called with the ENTIRE string left after consuming the delimiter, so
	/// you have to consume the list elements yourself!
	/// \par Example
	/// ```cpp
	/// consume_delimited_list("alpha, beta, gamma, omega ), blah, bleh", ",", ")" [](auto& sv) { println("'{}'", consume_while(sv, ascii::isalpha)); return true; });
	/// >> will print
	/// 'alpha'
	/// 'beta'
	/// 'gamma'
	/// 'omega'
	/// ```
	/// \param callback must be invocable as `callback(string_view&) -> bool`; if the callback returns false, consumption is stopped
	/// \returns `true` if closer is consumed; `false` if callback returned false at any point or if end-of-string is encountered before `closer` is consumed
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

	/// @}

	/// \internal
	/// TODO: Move splits to use std::ranges::(lazy_)split_view
	///       But check performance vs our split first!
	/// \endinternal

	/// \name Split Functions
	/// Functions that split strings into multiple parts, each delimited with some sort of delimiter.
	/// @{
	
	template <typename FUNC>
	requires (std::is_invocable_v<FUNC, std::string_view, bool> && !std::is_invocable_v<FUNC, std::string_view>)
	constexpr auto split_invoke(FUNC& func, std::string_view sv, bool is_final) noexcept(noexcept(func(std::string_view{}, true)))
	{
		return func(sv, is_final);
	}
	template <typename FUNC>
	requires std::is_invocable_v<FUNC, std::string_view>
	constexpr auto split_invoke(FUNC& func, std::string_view sv, bool is_final) noexcept(noexcept(func(std::string_view{})))
	{
		return func(sv);
	}

	/// Performs a basic "split" operation, calling `func` for each part of `source` delimited by `delim`.
	/// If no delimiters are found, calls `func` for entire string.
	/// \param func must be invocable as `func(string_view, bool)`; the second argument specifies if the given string is the final part of the split
	template <typename FUNC>
	constexpr void split(std::string_view source, char delim, FUNC&& func) noexcept(noexcept(split_invoke(func, std::string_view{}, true)))
	{
		size_t next = 0;
		while ((next = source.find_first_of(delim)) != std::string::npos)
		{
			split_invoke(func, source.substr(0, next), false);
			source.remove_prefix(next + 1);
		}
		split_invoke(func, source, true);
	}

	/// Performs a basic "split" operation, calling `func` for each part of `source` delimited by `delim`.
	/// If no delimiters are found, calls `func` for entire string.
	/// \param func must be invocable as `func(string_view, bool)`; the second argument specifies if the given string is the final part of the split
	template <typename FUNC>
	constexpr void split(std::string_view source, std::string_view delim, FUNC&& func) noexcept(noexcept(split_invoke(func, std::string_view{}, true)))
	{
		const size_t delim_size = delim.size();
		if (delim_size == 0) return;

		size_t next = 0;
		while ((next = source.find(delim)) != std::string::npos)
		{
			split_invoke(func, source.substr(0, next), false);
			source.remove_prefix(next + delim_size);
		}
		split_invoke(func, source, true);
	}

	/// Performs a basic "split" operation, calling `func` for each part of `source` delimited by any character in `delim`.
	/// If no delimiters are found, calls `func` for entire string.
	/// \param func must be invocable as `func(string_view, bool)`; the second argument specifies if the given string is the final part of the split
	template <typename FUNC>
	constexpr void split_on_any(std::string_view source, std::string_view delim, FUNC&& func) noexcept(noexcept(split_invoke(func, std::string_view{}, true)))
	{
		/// TODO: change `delim` type to isany-able
		if (delim.empty()) return;

		size_t next = 0;
		while ((next = source.find_first_of(std::string_view{ delim })) != std::string::npos)
		{
			split_invoke(func, source.substr(0, next), false);
			source.remove_prefix(next + 1);
		}
		split_invoke(func, source, true);
	}

	/// Performs a basic "split" operation, calling `func` for each part of `source` delimited by the `delim` function.
	/// If no delimiters are found, calls `func` for entire string.
	/// \par Example
	/// ```cpp
	/// split_on("alpha,beta;gamma", [](auto sv) { return sv.find_first_of(",;"); }), [](auto sv, bool) { println("'{}', sv); });
	/// >> will print
	/// 'alpha'
	/// 'beta'
	/// 'gamma'
	/// ```
	/// \param delim must be invocable as `delim(string_view) -> std::string::size_type`; 
	///		it should return the position in its parameter where the next delimiter is;
	///		if it returns `std::string::npos`, splitting ends there, calling `func` for the rest of the string
	/// \param func must be invocable as `func(string_view, bool)`; the second argument specifies if the given string is the final part of the split
	template <typename DELIM_FUNC, typename FUNC>
	requires std::is_invocable_r_v<size_t, DELIM_FUNC, std::string_view>
	inline void split_on(std::string_view source, DELIM_FUNC&& delim, FUNC&& func) noexcept(noexcept(split_invoke(func, std::string_view{}, true)) && noexcept(delim(std::string_view{})))
	{
		size_t start = 0;
		size_t end = 0;
		end = delim(source);
		while (end != std::string::npos)
		{
			split_invoke(func, source.substr(start, end - start), false);
			//source.remove_prefix(end);
			start = end;
			auto next = delim(source.substr(end + 1));
			if (next == std::string::npos)
				break;
			end = next + end + 1;
		}
		split_invoke(func, source.substr(start), true);
	}

	/// Does not include the character at `split_at` in the returned strings
	[[nodiscard]] constexpr std::pair<std::string_view, std::string_view> split_at(std::string_view src, size_t split_at) noexcept
	{
		if (split_at == std::string::npos)
			return { src, {} };
		return { src.substr(0, split_at), src.substr(split_at + 1) };
	}

	/// Does not include the character at `split_at` in the returned strings
	[[nodiscard]] constexpr bool split_at(std::string_view src, size_t split_at, std::string_view& first, std::string_view& second) noexcept
	{
		if (split_at == std::string::npos)
			return false;
		first = src.substr(0, split_at);
		second = src.substr(split_at + 1);
		return true;
	}

	/// Splits `src` once on the first instance of `delim`
	/// \returns a pair of string_views: the left and right parts of `src` split the first instance of `delim`; if no delim is found, returns `{ src, {} }`
	[[nodiscard]] constexpr std::pair<std::string_view, std::string_view> single_split(std::string_view src, char delim) noexcept
	{
		return split_at(src, src.find_first_of(delim));
	}

	/// Splits `src` once on the last instance of `delim`
	/// \returns a pair of string_views: the left and right parts of `src` split the first instance of `delim`; if no delim is found, returns `{ src, {} }`
	[[nodiscard]] constexpr std::pair<std::string_view, std::string_view> single_split_last(std::string_view src, char delim) noexcept
	{
		return split_at(src, src.find_last_of(delim));
	}

	/// Splits `src` once on the first instance of `delim`
	/// \returns whether delim was found
	/// \param first will be filled with the left part of the string, if delim is found
	/// \param second will be filled with the right part of the string, if delim is found
	[[nodiscard]] constexpr bool single_split(std::string_view src, char delim, std::string_view& first, std::string_view& second) noexcept
	{
		return split_at(src, src.find_first_of(delim), first, second);
	}

	/// Splits `src` once on the last instance of `delim`
	/// \returns whether delim was found
	/// \param first will be filled with the left part of the string, if delim is found
	/// \param second will be filled with the right part of the string, if delim is found
	[[nodiscard]] constexpr bool single_split_last(std::string_view src, char delim, std::string_view& first, std::string_view& second) noexcept
	{
		return split_at(src, src.find_last_of(delim), first, second);
	}

	/// Performs a more natural split of the string, that is: ignoring multiple delimiters in a row, and empty items
	/// \see split(std::string_view source, char delim, FUNC&& func)
	/// \par Example
	/// ```cpp
	/// split("these   are matched words ", ' ', [](auto sv, bool){ println("'{}'", sv); });
	/// >> will print
	/// 'these'
	/// 'are'
	/// 'matched'
	/// 'words'
	/// ```
	template <typename FUNC>
	inline void natural_split(std::string_view source, char delim, FUNC&& func) noexcept(noexcept(split_invoke(func, source, delim)))
	{
		size_t next = 0;
		while ((next = source.find_first_of(delim)) != std::string::npos)
		{
			split_invoke(func, source.substr(0, next), false);
			source.remove_prefix(next + 1);

			if ((next = source.find_first_not_of(delim)) == std::string::npos)
				return;

			source.remove_prefix(next);
		}

		if (!source.empty())
			split_invoke(func, source, true);
	}

	/// Performs a basic "split" operation, returning a `std::vector` of the split parts
	/// \see split(std::string_view source, char delim, FUNC&& func)
	/// \tparam RESULT_TYPE the type of the elements in the resulting vector; must be constructible from `string_view`
	template <typename RESULT_TYPE = std::string_view, string_or_char DELIM>
	[[nodiscard]] constexpr std::vector<RESULT_TYPE> split(std::string_view source, DELIM&& delim) noexcept
	{
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::split(source, std::forward<DELIM>(delim), [&result](std::string_view str) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}

	/// Performs a basic "split" operation, returning a `std::vector` of the split parts
	/// \see split_on_any(std::string_view source, std::string_view delim, FUNC&& func)
	/// \tparam RESULT_TYPE the type of the elements in the resulting vector; must be constructible from `string_view`
	template <typename RESULT_TYPE = std::string_view>
	[[nodiscard]] constexpr std::vector<RESULT_TYPE> split_on_any(std::string_view source, std::string_view delim) noexcept
	{
		/// TODO: change `delim` type to isany-able
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::split_on_any(source, delim, [&result](std::string_view str) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}


	/// Performs a basic "split" operation, returning a `std::vector` of the split parts
	/// \see split_on(std::string_view source, DELIM_FUNC&& delim, FUNC&& func)
	/// \tparam RESULT_TYPE the type of the elements in the resulting vector; must be constructible from `string_view`
	template <typename RESULT_TYPE = std::string_view, typename DELIM_FUNC>
	requires std::is_invocable_r_v<size_t, DELIM_FUNC, std::string_view>
	[[nodiscard]] std::vector<RESULT_TYPE> split_on(std::string_view source, DELIM_FUNC&& delim) noexcept(noexcept(delim(std::string_view{})))
	{
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::split_on(source, std::forward<DELIM_FUNC>(delim), [&result](std::string_view str) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}

	/// Performs a more natural split of the string, that is: ignoring multiple delimiters in a row, and empty items; returns a `std::vector` of the split parts
	/// \see natural_split(std::string_view source, char delim, FUNC&& func)
	template <typename RESULT_TYPE = std::string_view, string_or_char DELIM>
	[[nodiscard]] inline std::vector<RESULT_TYPE> natural_split(std::string_view source, DELIM&& delim) noexcept
	{
		std::vector<RESULT_TYPE> result;
		::ghassanpl::string_ops::natural_split(source, std::forward<DELIM>(delim), [&result](std::string_view str) {
			result.push_back(RESULT_TYPE{ str });
		});
		return result;
	}

	/// @}

	/// \name Join Functions
	/// Functions that join a range of formattable elements into a single string
	/// \note Formatting is done using stream operators (`operator<<`).
	/// \todo Use \c Stringification instead
	/// @{

	/// Returns a string that is created by joining together string representation of the elements in the `source` range.
	template <std::ranges::range T>
	[[nodiscard]] inline auto join(T&& source)
	{
		std::stringstream strm;
		for (auto&& p : std::forward<T>(source))
			strm << p;
		return strm.str();
	}

	/// Returns a string that is created by joining together string representation of the elements in the `source` range, separated by `delim`; `delim` is only added between elements.
	template <std::ranges::range T, string_or_char DELIM>
	[[nodiscard]] inline auto join(T&& source, DELIM const& delim)
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

	/// Returns a string that is created by joining together string representation of the elements in the `sources` ranges, separated by `delim`; `delim` is only added between elements.
	template <std::ranges::range... RANGES, string_or_char DELIM>
	[[nodiscard]] inline auto join_multiple(DELIM const& delim, RANGES&&... sources)
	{
		std::stringstream strm;
		bool first = true;
		([&]<typename RANGE>(RANGE && source) {
			for (auto&& p : std::forward<RANGE>(source))
			{
				if (!first) strm << delim;
				strm << p;
				first = false;
			}
		}(std::forward<RANGES>(sources)), ...);
		return strm.str();
	}


	/// Returns a string that is created by joining together string representation of the elements in the `source` range, separated by `delim`; `delim` is only added between elements; 
	/// the last element is delimited by `last_delim` instead of `delim`.
	template <std::ranges::range T, string_or_char DELIM, string_or_char LAST_DELIM>
	[[nodiscard]] inline auto join_and(T&& source, DELIM const& delim, LAST_DELIM&& last_delim)
	{
		using std::ranges::begin;
		using std::ranges::end;
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


	/// Same as \c join(T&& source, DELIM const& delim, LAST_DELIM&& last_delim)
	/// except each element is transformed by `transform_func` before being stringified and added to the result.
	/// \param transform_func must be invocable as `transform_fun(el)` for each element in the `source` range
	template <std::ranges::range T, string_or_char DELIM, string_or_char LAST_DELIM, typename FUNC>
	[[nodiscard]] inline auto join_and(T&& source, DELIM const& delim, LAST_DELIM&& last_delim, FUNC&& transform_func)
	{
		using std::ranges::begin;
		using std::ranges::end;
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
			strm << transform_func(*it);
			first = false;
		}
		return strm.str();
	}

	/// Same as \c join(T&& source, DELIM const& delim)
	/// except each element is transformed by `transform_func` before being stringified and added to the result.
	/// \param transform_func must be invocable as `transform_fun(el)` for each element in the `source` range
	template <std::ranges::range T, typename FUNC, string_or_char DELIM>
	[[nodiscard]] inline auto join(T&& source, DELIM const& delim, FUNC&& transform_func)
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

	/// @}
	
	template <string_or_char NEEDLE, typename FUNC>
	inline void find_all(std::string_view subject, NEEDLE&& search, FUNC&& func)
	{
		const auto search_sv = make_sv(search);

		if (search_sv.empty())
			return;

		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			func(subject.substr(pos, search_sv.size()));
			pos += search_sv.size();
		}
	}

	template <typename RESULT_TYPE = std::string_view, string_or_char NEEDLE>
	inline std::vector<RESULT_TYPE> find_all(std::string_view subject, NEEDLE&& search)
	{
		std::vector<RESULT_TYPE> result;
		find_all(subject, std::forward<NEEDLE>(search), [&](std::string_view sv) { result.push_back(RESULT_TYPE{ sv }); });
		return result;
	}

	/// \name Replace and Escape Functions
	/// \warning A lot of these functions have stupid and/or subtle bugs, or are not intuitive in their behavior.
	///		Use at your own risk. Pull requests welcome.
	/// \todo C++23's format has a string format specifications that automatically escape, use those when they become available
	/// @{

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

	template <string_or_char NEEDLE, string_or_char REPLACE>
	[[nodiscard]] inline std::string replaced(std::string subject, NEEDLE&& search, REPLACE&& replace)
	{
		string_ops::replace(subject, std::forward<NEEDLE>(search), std::forward<REPLACE>(replace));
		return subject;
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
	[[nodiscard]] inline std::string quoted(std::string&& subject, DELIMITER&& delimiter = '"', ESCAPE&& escape = '\\')
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
	requires std::is_invocable_v<ESCAPE_FUNC, std::string_view>&& std::is_constructible_v<std::string_view, std::invoke_result_t<ESCAPE_FUNC, std::string_view>>
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
	inline void escape(std::string& subject, std::string_view chars_to_escape, ESCAPE&& escape = '\\')
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

	template <typename ESCAPE_FUNC, typename ISPRINTABLE_FUNC = decltype(ascii::isprint)>
	inline void escape_non_printable(std::string& subject, ESCAPE_FUNC&& escape_func, ISPRINTABLE_FUNC&& isprintable_func = ascii::isprint)
	{
		static_assert(std::is_invocable_v<ESCAPE_FUNC, char> && std::is_constructible_v<std::string_view, std::invoke_result_t<ESCAPE_FUNC, char>>,
			"escape function must be invocable with (char) and must return something convertible to string_view");
		static_assert(std::is_invocable_v<ISPRINTABLE_FUNC, char> && std::is_constructible_v<bool, std::invoke_result_t<ISPRINTABLE_FUNC, char>>,
			"isprintable function must be a predicate invocable with (char)");

		size_t pos = 0;
		std::string::iterator it;
		while ((it = std::find_if_not(subject.begin() + pos, subject.end(), isprintable_func)) != subject.end())
		{
			pos = it - subject.begin();
			auto escape_str = escape_func(subject[pos]);
			subject.replace(pos, 1, escape_str);
			pos += escape_str.size();
		}
	}

	inline void escape_non_printable(std::string& subject)
	{
		escape_non_printable(subject, [](char c) {
			return std::format("\\x{:02x}", static_cast<uint8_t>(c));
		});
	}

	template <typename STR, string_or_char ESCAPE = char>
	[[nodiscard]] inline std::string escaped(STR&& subject, std::string_view to_escape = "\"\\", ESCAPE&& escape_str = '\\') /// Lint Note: Changing the initializer of to_escape to a R-string breaks doxygen
	{
		auto result = std::string{ subject };
		::ghassanpl::string_ops::escape(result, to_escape, std::forward<ESCAPE>(escape_str));
		return result;
	}

	template <typename STR>
	[[nodiscard]] inline std::string escaped_non_printable(STR&& subject)
	{
		auto result = std::string{ subject };
		::ghassanpl::string_ops::escape_non_printable(result);
		return result;
	}

	/// @}

	/// Returns a url-encoded version of the string
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

	/// Returns a url-decoded version of the string
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

	/// Gets a unique name based on `base_name`; `checker(str) -> bool` should return whether the given name is unique.
	/// Works by appending consecutive numbers and checking them.
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

#if !__cpp_lib_to_chars && !defined(DOXYGEN)

#else
	/// A version of `std::from_chars` that takes a `std::string_view` as the first argument
	template <std::integral T>
	[[nodiscard]] inline auto from_chars(std::string_view str, T& value, const int base = 10) noexcept {
		return std::from_chars(str.data(), str.data() + str.size(), value, base);
	}
	/// A version of `std::from_chars` that takes a `std::string_view` as the first argument
	template <std::floating_point T>
	[[nodiscard]] inline auto from_chars(std::string_view str, T& value, const std::chars_format chars_format = std::chars_format::general) noexcept {
		return std::from_chars(str.data(), str.data() + str.size(), value, chars_format);
	}
#endif

	/// A very basic "range" (not really a C++ range yet) that can be iterated over as if its a range of
	/// elements in `source` split by `split_on`.
	/// 
	/// \tparam SINGLE if false, we ignore consecutive delimiters
	/// 
	/// \todo Make this an actual range
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
				++*this;
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

		/// Returns the source string we're splitting
		auto source() const { return mSource; }
		/// Returns the character we're splitting on
		auto split_on() const { return mSplit; }

	private:

		std::string_view mSource;
		char mSplit;
	};

	// static_assert(std::ranges::range<split_range<true>>);

	/// Performs a basic word-wrapping split of `_source`, as if it was constrained to `max_width`.
	/// Splits will be performed implicitly on ' ' (or character boundaries if no other choice), and explictly on '\\n' characters
	/// \tparam T the type for the width values
	/// \param width_getter must be invocable as `width_getter(string_view) -> T` and should return the width of the given string
	///		calculating it however it deems appropriate
	/// \returns a vector containing a `string_view` for each line
	/// \todo example
	template <typename RESULT_TYPE = std::string_view, typename T, typename FUNC>
	requires std::is_arithmetic_v<T>&& std::is_invocable_r_v<T, FUNC, std::string_view>
	std::vector<RESULT_TYPE> word_wrap(std::string_view _source, T max_width, FUNC width_getter)
	{
		std::vector<RESULT_TYPE> result;

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
					result.push_back(RESULT_TYPE{ line_start, size_t(r.first - line_start) - 1 });
					space_left = max_width - word_width;
					line_start = r.first;
				}
				else
				{
					space_left -= width;
				}
			}

			result.push_back(RESULT_TYPE{ line_start, size_t(line.second - line_start) });
		}

		return result;
	}

	/// TODO: wildcard_match

	/// Word-wrapping function for constant-width characters.
	/// \see word_wrap(std::string_view _source, T max_width, FUNC width_getter)
	template <typename RESULT_TYPE = std::string_view, typename T>
	requires std::is_arithmetic_v<T>
	std::vector<RESULT_TYPE> word_wrap(std::string_view _source, T max_width, T letter_width)
	{
		return ::ghassanpl::string_ops::word_wrap<RESULT_TYPE>(_source, max_width, [letter_width](std::string_view str) { return T(str.size() * letter_width); });
	}

	inline size_t levenshtein_distance(std::string_view s1, std::string_view s2)
	{
		if (s1.size() > s2.size())
			std::swap(s1, s2);

		const auto min_size = s1.size();
		const auto max_size = s2.size();

		std::vector<size_t> lev_dist(min_size + 1);
		for (size_t i = 0; i < lev_dist.size(); ++i)
			lev_dist[i] = i;

		for (size_t j = 1; j <= max_size; ++j)
		{
			size_t previous_diagonal = lev_dist[0];
			size_t previous_diagonal_save{};
			++lev_dist[0];

			for (size_t i = 1; i <= min_size; ++i) 
			{
				previous_diagonal_save = lev_dist[i];
				if (s1[i - 1] == s2[j - 1])
					lev_dist[i] = previous_diagonal;
				else
					lev_dist[i] = std::min({ lev_dist[i - 1], lev_dist[i], previous_diagonal }) + 1;
				previous_diagonal = previous_diagonal_save;
			}
		}

		return lev_dist[min_size];
	}

	namespace detail
	{
		template <std::integral T>
		[[nodiscard]] inline auto string_to_number(std::string_view str, size_t* idx = nullptr, int base = 10)
		{
			const auto begin = str.data();
			auto end = str.data() + str.size();
			T value{};
			if (auto res = std::from_chars(begin, end, value, base); idx && res.ec == std::error_code{})
				*idx = size_t(res.ptr - begin);
			return value;
		}

		template <std::floating_point T>
		[[nodiscard]] inline auto string_to_number(std::string_view str, size_t* idx = nullptr, std::chars_format format = std::chars_format::general)
		{
			const auto begin = str.data();
			auto end = str.data() + str.size();
			T value{};
			if (auto res = std::from_chars(begin, end, value, format); idx && res.ec == std::error_code{})
				*idx = size_t(res.ptr - begin);
			return value;
		}
	}

	/// \name sto* replacements
	/// Functions equivalent to `std::stoi`, `std::stod`, etc that take `std::string_view` as its first argument
	/// @{
	[[nodiscard]] inline int stoi(std::string_view str, size_t* idx = nullptr, int base = 10) { return detail::string_to_number<int>(str, idx, base); }
	[[nodiscard]] inline long stol(std::string_view str, size_t* idx = nullptr, int base = 10) { return detail::string_to_number<long>(str, idx, base); }
	[[nodiscard]] inline long long stoll(std::string_view str, size_t* idx = nullptr, int base = 10) { return detail::string_to_number<long long>(str, idx, base); }
	[[nodiscard]] inline unsigned long stoul(std::string_view str, size_t* idx = nullptr, int base = 10) { return detail::string_to_number<unsigned long>(str, idx, base); }
	[[nodiscard]] inline unsigned long long stoull(std::string_view str, size_t* idx = nullptr, int base = 10) { return detail::string_to_number<unsigned long long>(str, idx, base); }
	[[nodiscard]] inline float stof(std::string_view str, size_t* idx = nullptr, std::chars_format format = std::chars_format::general) { return detail::string_to_number<float>(str, idx, format); }
	[[nodiscard]] inline double stod(std::string_view str, size_t* idx = nullptr, std::chars_format format = std::chars_format::general) { return detail::string_to_number<double>(str, idx, format); }
	[[nodiscard]] inline long double stold(std::string_view str, size_t* idx = nullptr, std::chars_format format = std::chars_format::general) { return detail::string_to_number<long double>(str, idx, format); }
	/// @}

	/// If `str` contains an integral number (and nothing else), returns that number, else an error-carrying `from_chars_result`.
	template <std::integral T = std::make_unsigned_t<size_t>>
	[[nodiscard]] inline auto to_number(std::string_view str, int base = 10) -> expected<T, std::from_chars_result> {
		const auto begin = str.data();
		auto end = str.data() + str.size();
		T value{};
		if (auto res = std::from_chars(begin, end, value, base); res.ec != std::error_code{} || res.ptr != end)
			return unexpected(res);
		return value;
	}

	/// If `str` contains an floating-point number (and nothing else), returns that number, else an error-carrying `from_chars_result`.
	template <std::floating_point T = std::float_t>
	[[nodiscard]] inline auto to_number(std::string_view str, std::chars_format format = std::chars_format::general) -> expected<T, std::from_chars_result> {
		const auto begin = str.data();
		auto end = str.data() + str.size();
		T value{};
		if (auto res = std::from_chars(begin, end, value, format); res.ec != std::error_code{} || res.ptr != end)
			return unexpected(res);
		return value;
	}

	template <typename CALLBACK>
	requires std::invocable<CALLBACK, size_t, std::string_view, std::string&>
	std::string callback_format(std::string_view fmt, CALLBACK&& callback)
	{
		/// TODO: Make this respect `}}` as an escaped `}`
		std::string result;
		size_t aid = 0;
		while (!fmt.empty())
		{
			auto text = consume_until(fmt, '{');
			result += text;
			if (fmt.empty())
				continue;
			std::ignore = consume(fmt, '{');

			if (consume(fmt, '{'))
			{
				result += '{';
				continue;
			}
			else
			{
				std::string fmt_clause_str = "{";
				auto fmt_clause = string_ops::consume_until_delim(fmt, '}');
				if (!string_ops::consume_at_end(fmt_clause, '}'))
					throw std::format_error("missing '}' in format string");
				if (!fmt_clause.empty())
				{
					auto num = string_ops::consume_until(fmt_clause, ':');
					if (!num.empty())
						aid = string_ops::stoull(num);
					fmt_clause_str += fmt_clause;
				}
				fmt_clause_str += '}';
				callback(aid, std::string_view{ fmt_clause_str }, result);
				++aid;
			}
		}
		return result;
	}

/// \showinitializer
#define GHPL_FORMAT_TEMPLATE typename... GHPL_ARGS
/// \showinitializer
#define GHPL_FORMAT_ARGS std::string_view ghpl_fmt, GHPL_ARGS&&... ghpl_args
/// \showinitializer
#define GHPL_FORMAT_FORWARD ghpl_fmt, std::forward<GHPL_ARGS>(ghpl_args)...
/// \showinitializer
#define GHPL_FORMAT_CALL std::vformat(ghpl_fmt, std::make_format_args(std::forward<GHPL_ARGS>(ghpl_args)...))
/// \showinitializer
#define GHPL_PRINT_CALL std::vprint_unicode(ghpl_fmt, std::make_format_args(std::forward<GHPL_ARGS>(ghpl_args)...))

	/// @}
}
