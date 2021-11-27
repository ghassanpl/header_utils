/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>
#include <algorithm>
#include <vector>
#include <sstream>
#include <charconv>

namespace ghassanpl::string_ops
{
	/// ///////////////////////////// ///
	/// ASCII functions
	/// ///////////////////////////// ///

	namespace ascii
	{

		/// Our own functions that do not block, are defined for values outside of uint8_t, and do not depend on locale
		[[nodiscard]] inline constexpr bool isalpha(char32_t cp) noexcept { return (cp >= 'A' && cp <= 'Z') || (cp >= 'a' && cp <= 'z'); }
		[[nodiscard]] inline constexpr bool isdigit(char32_t d)  noexcept { return d >= '0' && d <= '9'; }
		[[nodiscard]] inline constexpr bool isxdigit(char32_t d) noexcept { return (d >= '0' && d <= '9') || (d >= 'A' && d <= 'F') || (d >= 'a' && d <= 'f'); }
		[[nodiscard]] inline constexpr bool isalnum(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isdigit(cp) || ::ghassanpl::string_ops::ascii::isalpha(cp); }
		[[nodiscard]] inline constexpr bool isident(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isdigit(cp) || ::ghassanpl::string_ops::ascii::isalpha(cp) || cp == '_'; }
		[[nodiscard]] inline constexpr bool isspace(char32_t d)  noexcept { return d == ' ' || d == '\t' || d == '\n' || d == '\v' || d == '\r' || d == '\f'; }
		[[nodiscard]] inline constexpr bool ispunct(char32_t d)  noexcept { return (d >= 33 && d <= 47) || (d >= 58 && d <= 64) || (d >= 91 && d <= 96) || (d >= 123 && d <= 126); }
		[[nodiscard]] inline constexpr bool islower(char32_t cp) noexcept { return (cp >= 'a' && cp <= 'z'); }
		[[nodiscard]] inline constexpr bool isupper(char32_t cp) noexcept { return (cp >= 'A' && cp <= 'Z'); }
		[[nodiscard]] inline constexpr bool iscntrl(char32_t cp) noexcept { return cp == 0x7F || cp < 0x20; }
		[[nodiscard]] inline constexpr bool isblank(char32_t cp) noexcept { return cp == ' ' || cp == '\t'; }
		[[nodiscard]] inline constexpr bool isgraph(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isalnum(cp) || ::ghassanpl::string_ops::ascii::ispunct(cp); }
		[[nodiscard]] inline constexpr bool isprint(char32_t cp) noexcept { return ::ghassanpl::string_ops::ascii::isgraph(cp) || cp == ' '; }

		[[nodiscard]] inline constexpr char32_t toupper(char32_t cp) noexcept { return (cp >= 'a' && cp <= 'z') ? (cp ^ 0b100000) : cp; }
		[[nodiscard]] inline constexpr char32_t tolower(char32_t cp) noexcept { return (cp >= 'A' && cp <= 'Z') ? (cp | 0b100000) : cp; }

		[[nodiscard]] inline std::string tolower(std::string str) noexcept { std::for_each(str.begin(), str.end(), [](char& cp) { cp = (char)::ghassanpl::string_ops::ascii::tolower(cp); }); return str; }
		[[nodiscard]] inline std::string tolower(std::string_view str) noexcept { return ::ghassanpl::string_ops::ascii::tolower(std::string{str}); }
		[[nodiscard]] inline std::string tolower(const char* str) noexcept { return ::ghassanpl::string_ops::ascii::tolower(std::string{ str }); }

		[[nodiscard]] inline std::string toupper(std::string str) noexcept { std::for_each(str.begin(), str.end(), [](char& cp) { cp = (char)::ghassanpl::string_ops::ascii::toupper(cp); }); return str; }
		[[nodiscard]] inline std::string toupper(std::string_view str) noexcept { return ::ghassanpl::string_ops::ascii::toupper(std::string{str}); }
		[[nodiscard]] inline std::string toupper(const char* str) noexcept { return ::ghassanpl::string_ops::ascii::toupper(std::string{str}); }

		/// todigit is defined only for values between 0 and 9
		[[nodiscard]] inline constexpr char32_t todigit(int v) noexcept { return char32_t(v) + U'0'; }
		/// toxdigit is defined only for values between 0 and 15
		[[nodiscard]] inline constexpr char32_t toxdigit(int v) noexcept { return (v > 9) ? (char32_t(v - 10) + U'A') : (char32_t(v) + U'0'); }

		[[nodiscard]] constexpr bool strings_equal_ignore_case(std::string_view a, std::string_view b)
		{
			return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return ::ghassanpl::string_ops::ascii::toupper(a) == ::ghassanpl::string_ops::ascii::toupper(b); });
		}

		[[nodiscard]] constexpr bool lexicographical_compare_ignore_case(std::string_view a, std::string_view b)
		{
			return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return ::ghassanpl::string_ops::ascii::toupper(a) < ::ghassanpl::string_ops::ascii::toupper(b); });
		}
	}

	/// ///////////////////////////// ///
	/// Pre-C++23 stuff
	/// ///////////////////////////// ///
	
	bool contains(std::string_view str, char c)
	{
#if __cpp_lib_string_contains
		return str.contains(c);
#else
		return ::std::find(str.begin(), str.end(), c) != str.end();
#endif
	}

	/// ///////////////////////////// ///
	/// Makes
	/// ///////////////////////////// ///

	[[nodiscard]] inline constexpr std::string_view make_sv(const char* start, const char* end) noexcept { return std::string_view{ start, static_cast<size_t>(end - start) }; }
	//template <std::contiguous_iterator IT, typename T = typename std::iterator_traits<IT>::value_type>
	template <typename IT, typename T = typename std::iterator_traits<IT>::value_type>
	[[nodiscard]] inline constexpr std::string_view make_sv(IT start, IT end) { return std::string_view{ std::to_address(start), static_cast<size_t>(std::distance(std::to_address(start), std::to_address(end))) }; }

	[[nodiscard]] inline std::string make_string(const char* start, const char* end) { return std::string{ start, static_cast<size_t>(end - start) }; }
	//template <std::contiguous_iterator IT, typename T = typename std::iterator_traits<IT>::value_type>
	template <typename IT, typename T = typename std::iterator_traits<IT>::value_type>
	[[nodiscard]] inline std::string make_string(IT start, IT end) { return std::string{ std::to_address(start), static_cast<size_t>(std::distance(std::to_address(start), std::to_address(end))) }; }

	/// for predicates
	[[nodiscard]] inline std::string to_string(std::string_view from) noexcept { return std::string{ from }; }

	/// ///////////////////////////// ///
	/// Trims
	/// ///////////////////////////// ///

	[[nodiscard]] inline std::string_view trimmed_whitespace_right(std::string_view str) noexcept { return make_sv(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base()); }
	[[nodiscard]] inline std::string_view trimmed_whitespace_left(std::string_view str) noexcept { return make_sv(std::find_if_not(str.begin(), str.end(), ::ghassanpl::string_ops::ascii::isspace), str.end()); }
	[[nodiscard]] inline std::string_view trimmed_whitespace(std::string_view str) noexcept { return trimmed_whitespace_left(trimmed_whitespace_right(str)); }
	[[nodiscard]] inline std::string_view trimmed_until(std::string_view str, char chr) noexcept { return make_sv(std::find(str.begin(), str.end(), chr), str.end()); }

	inline void trim_whitespace_right(std::string_view& str) noexcept { str = make_sv(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ::ghassanpl::string_ops::ascii::isspace).base()); }
	inline void trim_whitespace_left(std::string_view& str) noexcept { str = make_sv(std::find_if_not(str.begin(), str.end(), ::ghassanpl::string_ops::ascii::isspace), str.end()); }
	inline void trim_whitespace(std::string_view& str) noexcept { trim_whitespace_left(str); trim_whitespace_right(str); }

	template <typename FUNC>
	[[nodiscard]] inline std::string_view trimmed_while(std::string_view str, FUNC&& func) noexcept { return make_sv(std::find_if_not(str.begin(), str.end(), std::forward<FUNC>(func)), str.end()); }

	/// ///////////////////////////// ///
	/// Consume
	/// ///////////////////////////// ///
	
	inline char consume(std::string_view& str)
	{
		if (str.empty())
			return {};
		const auto result = str[0];
		str.remove_prefix(1);
		return result;
	}

	inline bool consume(std::string_view& str, char val)
	{
		if (str.starts_with(val))
		{
			str.remove_prefix(1);
			return true;
		}
		return false;
	}

	inline bool consume(std::string_view& str, std::string_view val)
	{
		if (str.starts_with(val))
		{
			str.remove_prefix(val.size());
			return true;
		}
		return false;
	}

	/*
	template <size_t N>
	bool consume(string_view& str, char(&val)[N])
	{
		return consume(str, make_sv(val, val + N));
	}
	*/
	
	inline bool consume_at_end(std::string_view& str, char val)
	{
		if (str.ends_with(val))
		{
			str.remove_prefix(1);
			return true;
		}
		return false;
	}

	inline bool consume_at_end(std::string_view& str, std::string_view val)
	{
		if (str.ends_with(val))
		{
			str.remove_suffix(val.size());
			return true;
		}
		return false;
	}

	template <typename FUNC>
	//requires std::invocable<FUNC, char>
	inline std::string_view consume_while(std::string_view& str, FUNC&& pred)
	{
		const auto start = str.begin();
		while (!str.empty() && pred(str[0]))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	template <typename FUNC>
	inline std::string_view consume_while(std::string_view& str, char c)
	{
		const auto start = str.begin();
		while (str.starts_with(c))
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	inline std::string_view consume_until(std::string_view& str, char c)
	{
		const auto start = str.begin();
		while (!str.empty() && str[0] != c)
			str.remove_prefix(1);
		return make_sv(start, str.begin());
	}

	inline std::string_view consume_n(std::string_view& str, size_t n)
	{
		n = std::min(str.size(), n);
		auto result = str.substr(0, n);
		str.remove_prefix(n);
		return result;
	}

	template <typename CALLBACK>
	inline bool consume_delimited_list_non_empty(std::string_view& str, std::string_view delimiter, CALLBACK callback)
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
	inline bool consume_delimited_list(std::string_view& str, std::string_view delimiter, std::string_view closer, CALLBACK callback)
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

	inline std::string_view consume_c_identifier(std::string_view& str)
	{
		if (str.empty() || !(ascii::isalpha(str[0]) || str[0] == '_'))
			return {};

		const auto start = str.begin();
		str.remove_prefix(1);
		consume_while(str, ascii::isident);
		return make_sv(start, str.begin());
	}

	inline std::string_view consume_c_identifier_with(std::string_view& str, std::string_view additional_chars)
	{
		if (str.empty() || !(ascii::isalpha(str[0]) || str[0] == '_' || contains(additional_chars, str[0])))
			return {};

		const auto start = str.begin();
		str.remove_prefix(1);
		consume_while(str, [additional_chars](char c) { return ascii::isident(c) || contains(additional_chars, c); });
		return make_sv(start, str.begin());
	}


#if _MSC_VER
	inline std::pair<std::string_view, double> consume_c_float(std::string_view& str)
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

	inline std::pair<std::string_view, int64_t> consume_c_integer(std::string_view& str, int base = 10)
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

	inline std::pair<std::string_view, int64_t> consume_c_unsigned(std::string_view& str, int base = 10)
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

	size_t append_utf8(std::string& buffer, char32_t cp);

	template <char DELIMITER = '\''>
	inline std::pair<std::string_view, std::string> consume_c_string(std::string_view& strv)
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

	/// TODO: this 
	void consume_c_literal(std::string_view& str);

#endif

	/// ///////////////////////////// ///
	/// Basic UTF-8 stuff
	/// ///////////////////////////// ///
	
	/// Assuming str is valid UTF-8
	[[gsl::suppress(type.1, es.79)]]
	inline char32_t consume_utf8(std::string_view& str)
	{
		if (str.empty()) return 0;
		auto it = (uint8_t*)std::to_address(str.begin());
		char32_t cp = *it;

		int length = 0;
		if (cp < 0x80) length = 1;
		else if ((cp >> 5) == 0x6)  length = 2;
		else if ((cp >> 4) == 0xe)  length = 3;
		else if ((cp >> 3) == 0x1e) length = 4;
		else return 0;

		switch (length) {
		case 2:
			++it; cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
			break;
		case 3:
			++it; cp = ((cp << 12) & 0xffff) + ((*it << 6) & 0xfff);
			++it; cp += (*it) & 0x3f;
			break;
		case 4:
			++it; cp = ((cp << 18) & 0x1fffff) + (((*it) << 12) & 0x3ffff);
			++it; cp += ((*it) << 6) & 0xfff;
			++it; cp += (*it) & 0x3f;
			break;
		}
		str.remove_prefix(length);
		return cp;
	}

	/// Assuming codepoint is valid
	[[gsl::suppress(type.1)]]
	inline size_t append_utf8(std::string& buffer, char32_t cp)
	{
		if (cp < 0x80)
		{
			buffer += static_cast<char>(cp);
			return 1;
		}
		else if (cp < 0x800)
		{
			buffer += static_cast<char>((cp >> 6) | 0xc0);
			buffer += static_cast<char>((cp & 0x3f) | 0x80);
			return 2;
		}
		else if (cp < 0x10000)
		{
			buffer += static_cast<char>((cp >> 12) | 0xe0);
			buffer += static_cast<char>(((cp >> 6) & 0x3f) | 0x80);
			buffer += static_cast<char>((cp & 0x3f) | 0x80);
			return 3;
		}
		else
		{
			buffer += static_cast<char>((cp >> 18) | 0xf0);
			buffer += static_cast<char>(((cp >> 12) & 0x3f) | 0x80);
			buffer += static_cast<char>(((cp >> 6) & 0x3f) | 0x80);
			buffer += static_cast<char>((cp & 0x3f) | 0x80);
			return 4;
		}
	}

	[[gsl::suppress(type.1)]]
	/// Assumes codepoint is valid
	inline std::string to_utf8(char32_t cp)
	{
		if (cp < 0x80)
			return { static_cast<char>(cp) };
		else if (cp < 0x800)
			return { static_cast<char>((cp >> 6) | 0xc0), static_cast<char>((cp & 0x3f) | 0x80) };
		else if (cp < 0x10000)
			return { static_cast<char>((cp >> 12) | 0xe0), static_cast<char>(((cp >> 6) & 0x3f) | 0x80), static_cast<char>((cp & 0x3f) | 0x80) };
		else
			return { static_cast<char>((cp >> 18) | 0xf0), static_cast<char>(((cp >> 12) & 0x3f) | 0x80), static_cast<char>(((cp >> 6) & 0x3f) | 0x80), static_cast<char>((cp & 0x3f) | 0x80) };
	}

	template <std::ranges::input_range R>
	requires std::ranges::view<R>
	struct utf8_view : public std::ranges::view_interface<utf8_view<R>>
	{
		template <typename RANGE_ITER, typename SENTINEL>
		struct utf8_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = char32_t;
			using difference_type = ptrdiff_t;
			using reference = char32_t;

			constexpr utf8_iterator(RANGE_ITER current, SENTINEL end) : mCurrent(std::move(current)), mEnd(std::move(end)) {}

			constexpr value_type operator*() const { 
				const auto length = len();
				auto it = mCurrent;
				char32_t cp = std::bit_cast<uint8_t>(*it);

				switch (length) {
				case 2:
					++it; cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
					break;
				case 3:
					++it; cp = ((cp << 12) & 0xffff) + ((*it << 6) & 0xfff);
					++it; cp += (*it) & 0x3f;
					break;
				case 4:
					++it; cp = ((cp << 18) & 0x1fffff) + (((*it) << 12) & 0x3ffff);
					++it; cp += ((*it) << 6) & 0xfff;
					++it; cp += (*it) & 0x3f;
					break;
				}
				return cp;
			}

			constexpr utf8_iterator& operator++() {
				std::advance(mCurrent, len());
				return *this;
			}

			constexpr utf8_iterator operator++(int) {
				auto copy = *this;
				++*this;
				return copy;
			}

			constexpr auto operator<=>(utf8_iterator const&) const noexcept = default;
			constexpr bool operator==(utf8_iterator const&) const noexcept = default;
			constexpr bool operator!=(utf8_iterator const&) const noexcept = default;

		private:

			size_t len() const
			{
				if (mCurrent >= mEnd)
					throw std::out_of_range("utf8 iterator out of range");

				const unsigned cp = std::bit_cast<uint8_t>(*mCurrent);
				size_t length = 0;
				if (cp < 0x80) length = 1;
				else if ((cp >> 5) == 0x6) length = 2;
				else if ((cp >> 4) == 0xe) length = 3;
				else if ((cp >> 3) == 0x1e) length = 4;
				else
					throw std::runtime_error("invalid utf-8 prefix");

				if (mCurrent + length > mEnd)
					throw std::runtime_error("utf-8 range contains codepoint with length beyond end of range");

				return length;
			}

			RANGE_ITER mCurrent;
			SENTINEL mEnd;
		};

		utf8_view() = default;

		constexpr utf8_view(R base)
			: mBase(base)
		{
		}

		constexpr R base() const&
		{
			return mBase;
		}
		constexpr R base() &&
		{
			return std::move(mBase);
		}

		constexpr auto begin() const
		{
			return utf8_iterator<decltype(std::begin(mBase)), decltype(std::end(mBase))>{std::begin(mBase), std::end(mBase)};
		}

		constexpr auto end() const
		{
			return utf8_iterator<decltype(std::end(mBase)), decltype(std::end(mBase))>{std::end(mBase), std::end(mBase)};
		}

	private:
		R mBase{};
	};

	//template<class R> custom_take_view(R&& base, std::iter_difference_t<rg::iterator_t<R>>) ->custom_take_view<rg::views::all_t<R>>;

	/// ///////////////////////////// ///
	/// Other
	/// ///////////////////////////// ///

	template <typename DELIM, typename FUNC>
	inline void split(std::string_view source, DELIM&& delim, FUNC&& func) noexcept
	{
		size_t next = 0;
		while ((next = source.find_first_of(delim)) != std::string::npos)
		{
			func(source.substr(0, next), false);
			source.remove_prefix(next + 1);
		}
		func(source, true);
	}

	template <typename DELIM, typename FUNC>
	inline void natural_split(std::string_view source, DELIM&& delim, FUNC&& func) noexcept
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

	template <typename DELIM>
	[[nodiscard]] inline std::vector<std::string_view> split(std::string_view source, DELIM&& delim) noexcept
	{
		std::vector<std::string_view> result;
		::ghassanpl::string_ops::split(source, std::forward<DELIM>(delim), [&result](std::string_view str, bool last) {
			result.push_back(str);
		});
		return result;
	}

	template <typename DELIM>
	[[nodiscard]] inline std::vector<std::string_view> natural_split(std::string_view source, DELIM&& delim) noexcept
	{
		std::vector<std::string_view> result;
		::ghassanpl::string_ops::natural_split(source, std::forward<DELIM>(delim), [&result](std::string_view str, bool last) {
			result.push_back(str);
		});
		return result;
	}

#if _MSC_VER
	template <std::ranges::range T, typename DELIM>
#else
	template <typename T, typename DELIM>
#endif
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

#if _MSC_VER
	template <std::ranges::range T, typename DELIM, typename LAST_DELIM>
#else
	template <typename T, typename DELIM, typename LAST_DELIM>
#endif
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

#if _MSC_VER
	template <std::ranges::range T, typename FUNC, typename DELIM>
#else
	template <typename T, typename FUNC, typename DELIM>
#endif
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

	template <typename NEEDLE, typename REPLACE>
	inline void replace(std::string& subject, NEEDLE&& search, REPLACE&& replace)
	{
		using std::empty;
		using std::size;

		if (std::string_view{ search }.empty())
			return;

		const auto search_size = std::string_view{ search }.size();
		const auto replace_size = std::string_view{ replace }.size();

		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos)
		{
			subject.replace(pos, search_size, replace);
			pos += replace_size;
		}
	}


#if !_MSC_VER

#else
	template <typename T>
	inline auto from_chars(std::string_view str, T& value, const int base = 10) noexcept {
		return std::from_chars(str.data(), str.data() + str.size(), value, base);
	}
#endif

}
