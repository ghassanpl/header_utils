#pragma once
/// Based on https://github.com/martinmoene/string-view-lite
/// Distributed under the Boost Software License, Version 1.0.

#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

struct string_view
{
public:

	typedef std::char_traits<char> traits_type;
	typedef char value_type;

	typedef char* pointer;
	typedef char const* const_pointer;
	typedef char& reference;
	typedef char const& const_reference;

	typedef const_pointer iterator;
	typedef const_pointer const_iterator;
	typedef std::reverse_iterator< const_iterator > reverse_iterator;
	typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

	typedef std::size_t     size_type;
	typedef std::ptrdiff_t  difference_type;

	string_view() noexcept
		: data_(nullptr)
		, size_(0)
	{}

	string_view(string_view const& other) noexcept = default;

	string_view(char const* s, size_type count) noexcept
		: data_(s)
		, size_(count)
	{}

	string_view(char const* s) noexcept
		: data_(s)
		, size_(traits_type::length(s))
	{}

	string_view(char const* s, char const* e) noexcept
		: data_(s)
		, size_(e - s)
	{}

	string_view(std::nullptr_t) noexcept = delete;

	string_view& operator=(string_view const& other) noexcept = default;

	const_iterator begin()  const noexcept { return data_; }
	const_iterator end()    const noexcept { return data_ + size_; }

	const_iterator cbegin() const noexcept { return begin(); }
	const_iterator cend()   const noexcept { return end(); }

	const_reverse_iterator rbegin()  const noexcept { return const_reverse_iterator(end()); }
	const_reverse_iterator rend()    const noexcept { return const_reverse_iterator(begin()); }

	const_reverse_iterator crbegin() const noexcept { return rbegin(); }
	const_reverse_iterator crend()   const noexcept { return rend(); }

	size_type size()     const noexcept { return size_; }
	size_type length()   const noexcept { return size_; }
	size_type max_size() const noexcept { return (std::numeric_limits< size_type >::max)(); }

	bool empty() const noexcept
	{
		return 0 == size_;
	}

	const_reference operator[](size_type pos) const
	{
		return data_at(pos);
	}

	const_reference at(size_type pos) const
	{
		if (pos >= size())
			throw std::out_of_range("nonstring_view::at()");
		return data_at(pos);
	}

	const_reference front() const { return data_at(0); }
	const_reference back()  const { return data_at(size() - 1); }

	const_pointer   data()  const noexcept { return data_; }
	const_pointer   data_end()  const noexcept { return data_ + size_; }

	void remove_prefix(size_type n)
	{
		assert(n <= size());
		data_ += n;
		size_ -= n;
	}

	void remove_suffix(size_type n)
	{
		assert(n <= size());
		size_ -= n;
	}

	void swap(string_view& other) noexcept
	{
		const string_view tmp(other);
		other = *this;
		*this = tmp;
	}

	size_type copy(char* dest, size_type n, size_type pos = 0) const
	{
		if (pos > size())
			throw std::out_of_range("nonstring_view::copy()");

		const size_type rlen = (std::min)(n, size() - pos);
		(void)traits_type::copy(dest, data() + pos, rlen);
		return rlen;
	}

	string_view substr(size_type pos = 0, size_type n = npos) const
	{
		if (pos > size())
			throw std::out_of_range("nonstring_view::substr()");
		return string_view{ data() + pos, (std::min)(n, size() - pos) };
	}

	// compare(), 6x:

	int compare(string_view other) const noexcept // (1)
	{
		if (const int result = traits_type::compare(data(), other.data(), (std::min)(size(), other.size())))
			return result;

		return size() == other.size() ? 0 : size() < other.size() ? -1 : 1;
	}

	int compare(size_type pos1, size_type n1, string_view other) const // (2)
	{
		return substr(pos1, n1).compare(other);
	}

	int compare(size_type pos1, size_type n1, string_view other, size_type pos2, size_type n2) const // (3)
	{
		return substr(pos1, n1).compare(other.substr(pos2, n2));
	}

	int compare(char const* s) const
	{
		return compare(string_view{ s });
	}

	int compare(size_type pos1, size_type n1, char const* s) const
	{
		return substr(pos1, n1).compare(string_view{ s });
	}

	int compare(size_type pos1, size_type n1, char const* s, size_type n2) const
	{
		return substr(pos1, n1).compare(string_view{ s, n2 });
	}

	bool starts_with(string_view v) const noexcept
	{
		return size() >= v.size() && compare(0, v.size(), v) == 0;
	}

	bool starts_with(char c) const noexcept
	{
		return starts_with(string_view{ &c, 1 });
	}

	bool starts_with(char const* s) const
	{
		return starts_with(string_view{ s });
	}

	bool ends_with(string_view v) const noexcept
	{
		return size() >= v.size() && compare(size() - v.size(), npos, v) == 0;
	}

	bool ends_with(char c) const noexcept
	{
		return ends_with(string_view{ &c, 1 });
	}

	bool ends_with(char const* s) const
	{
		return ends_with(string_view{ s });
	}

	size_type find(string_view v, size_type pos = 0) const noexcept
	{
		return assert(v.size() == 0 || v.data() != nullptr)
			, pos >= size()
			? npos : to_pos(
				std::search(cbegin() + pos, cend(), v.cbegin(), v.cend(), traits_type::eq)
			);
	}

	size_type find(char c, size_type pos = 0) const noexcept
	{
		return find(string_view{ &c, 1 }, pos);
	}

	size_type find(char const* s, size_type pos, size_type n) const
	{
		return find(string_view{ s, n }, pos);
	}

	size_type find(char const* s, size_type pos = 0) const
	{
		return find(string_view{ s }, pos);
	}

	size_type rfind(string_view v, size_type pos = npos) const noexcept
	{
		if (size() < v.size())
		{
			return npos;
		}

		if (v.empty())
		{
			return (std::min)(size(), pos);
		}

		const_iterator last = cbegin() + (std::min)(size() - v.size(), pos) + v.size();
		const_iterator result = std::find_end(cbegin(), last, v.cbegin(), v.cend(), traits_type::eq);

		return result != last ? size_type(result - cbegin()) : npos;
	}

	size_type rfind(char c, size_type pos = npos) const noexcept
	{
		return rfind(string_view{ &c, 1 }, pos);
	}

	size_type rfind(char const* s, size_type pos, size_type n) const
	{
		return rfind(string_view{ s, n }, pos);
	}

	size_type rfind(char const* s, size_type pos = npos) const
	{
		return rfind(string_view{ s }, pos);
	}

	size_type find_first_of(string_view v, size_type pos = 0) const noexcept
	{
		return pos >= size()
			? npos
			: to_pos(std::find_first_of(cbegin() + pos, cend(), v.cbegin(), v.cend(), traits_type::eq));
	}

	size_type find_first_of(char c, size_type pos = 0) const noexcept
	{
		return find_first_of(string_view{ &c, 1 }, pos);
	}

	size_type find_first_of(char const* s, size_type pos, size_type n) const
	{
		return find_first_of(string_view{ s, n }, pos);
	}

	size_type find_first_of(char const* s, size_type pos = 0) const
	{
		return find_first_of(string_view{ s }, pos);
	}

	size_type find_last_of(string_view v, size_type pos = npos) const noexcept
	{
		return empty()
			? npos
			: pos >= size()
			? find_last_of(v, size() - 1)
			: to_pos(std::find_first_of(const_reverse_iterator(cbegin() + pos + 1), crend(), v.cbegin(), v.cend(), traits_type::eq));
	}

	size_type find_last_of(char c, size_type pos = npos) const noexcept
	{
		return find_last_of(string_view{ &c, 1 }, pos);
	}

	size_type find_last_of(char const* s, size_type pos, size_type count) const
	{
		return find_last_of(string_view{ s, count }, pos);
	}

	size_type find_last_of(char const* s, size_type pos = npos) const
	{
		return find_last_of(string_view{ s }, pos);
	}

	size_type find_first_not_of(string_view v, size_type pos = 0) const noexcept
	{
		return pos >= size()
			? npos
			: to_pos(std::find_if(cbegin() + pos, cend(), not_in_view(v)));
	}

	size_type find_first_not_of(char c, size_type pos = 0) const noexcept
	{
		return find_first_not_of(string_view{ &c, 1 }, pos);
	}

	size_type find_first_not_of(char const* s, size_type pos, size_type count) const
	{
		return find_first_not_of(string_view{ s, count }, pos);
	}

	size_type find_first_not_of(char const* s, size_type pos = 0) const
	{
		return find_first_not_of(string_view{ s }, pos);
	}

	size_type find_last_not_of(string_view v, size_type pos = npos) const noexcept
	{
		return empty()
			? npos
			: pos >= size()
			? find_last_not_of(v, size() - 1)
			: to_pos(std::find_if(const_reverse_iterator(cbegin() + pos + 1), crend(), not_in_view(v)));
	}

	size_type find_last_not_of(char c, size_type pos = npos) const noexcept
	{
		return find_last_not_of(string_view{ &c, 1 }, pos);
	}

	size_type find_last_not_of(char const* s, size_type pos, size_type count) const
	{
		return find_last_not_of(string_view{ s, count }, pos);
	}

	size_type find_last_not_of(char const* s, size_type pos = npos) const
	{
		return find_last_not_of(string_view{ s }, pos);
	}

	bool contains(const string_view right) const noexcept { return find(right) != npos; }
	bool contains(const char right) const noexcept { return find(right) != npos; }
	bool contains(const char* const right) const { return find(right) != npos; }

	enum : size_type { npos = size_type(-1) };

private:
	struct not_in_view
	{
		const string_view& v;

		explicit not_in_view(string_view& v_) : v(v_) {}

		bool operator()(char c) const
		{
			return npos == v.find_first_of(c);
		}
	};

	size_type to_pos(const_iterator it) const
	{
		return it == cend() ? npos : size_type(it - cbegin());
	}

	size_type to_pos(const_reverse_iterator it) const
	{
		return it == crend() ? npos : size_type(crend() - it - 1);
	}

	const_reference data_at(size_type pos) const
	{
		return assert(pos < size()), data_[pos];
	}

private:
	const_pointer data_;
	size_type     size_;

public:

	string_view(std::string const& s) noexcept
		: data_(s.data())
		, size_(s.size())
	{}

	explicit operator std::string() const
	{
		return to_string();
	}

	std::string to_string() const
	{
		return std::string(begin(), end());
	}

	/// Checks if this string_view is a true subset of `bigger_string` (true subset meaning they view over overlapping memory subregions)
	bool is_inside(string_view bigger_string) const
	{
		return bigger_string.data() - this->data() >= 0 && this->data_end() - bigger_string.data_end() >= 0;
	}
};

inline bool operator== (string_view lhs, string_view rhs) noexcept { return lhs.size() == rhs.size() && lhs.compare(rhs) == 0; }
inline bool operator!= (string_view lhs, string_view rhs) noexcept { return !(lhs == rhs); }
inline bool operator< (string_view lhs, string_view rhs) noexcept { return lhs.compare(rhs) < 0; }
inline bool operator<= (string_view lhs, string_view rhs) noexcept { return lhs.compare(rhs) <= 0; }
inline bool operator> (string_view lhs, string_view rhs) noexcept { return lhs.compare(rhs) > 0; }
inline bool operator>= (string_view lhs, string_view rhs) noexcept { return lhs.compare(rhs) >= 0; }

inline std::string to_string(string_view v) { return { v.begin(), v.end() }; }

/// \defgroup ASCII ASCII
/// These functions operate on codepoints and strings encoded as ASCII.
/// @{
namespace ascii
{

	/// \name is* and to* functions
	/// These are our own versions of \<cctype\> functions that do not block, are defined (false) for values outside of uint8_t, and do not depend on locale (plus you can take pointers to them).
	/// @{

	constexpr bool isalpha(char32_t cp) noexcept { return (cp >= 65 && cp <= 90) || (cp >= 97 && cp <= 122); }
	constexpr bool isdigit(char32_t cp) noexcept { return cp >= 48 && cp <= 57; }
	constexpr bool isodigit(char32_t cp) noexcept { return cp >= 48 && cp <= 55; }
	constexpr bool isxdigit(char32_t d) noexcept { return (d >= 48 && d <= 57) || (d >= 65 && d <= 70) || (d >= 97 && d <= 102); }
	constexpr bool isalnum(char32_t cp) noexcept { return ascii::isdigit(cp) || ascii::isalpha(cp); }
	constexpr bool isident(char32_t cp) noexcept { return ascii::isdigit(cp) || ascii::isalpha(cp) || cp == 95; }
	constexpr bool isidentstart(char32_t cp) noexcept { return ascii::isalpha(cp) || cp == 95; }
	constexpr bool isspace(char32_t cp) noexcept { return (cp >= 9 && cp <= 13) || cp == 32; }
	constexpr bool ispunct(char32_t cp) noexcept { return (cp >= 33 && cp <= 47) || (cp >= 58 && cp <= 64) || (cp >= 91 && cp <= 96) || (cp >= 123 && cp <= 126); }
	constexpr bool islower(char32_t cp) noexcept { return cp >= 97 && cp <= 122; }
	constexpr bool isupper(char32_t cp) noexcept { return cp >= 65 && cp <= 90; }
	constexpr bool iscntrl(char32_t cp) noexcept { return cp == 0x7F || cp < 0x20; }
	constexpr bool isblank(char32_t cp) noexcept { return cp == 32 || cp == 9; }
	constexpr bool isgraph(char32_t cp) noexcept { return cp >= 33 && cp <= 126; }
	constexpr bool isprint(char32_t cp) noexcept { return cp >= 32 && cp <= 126; }

	constexpr char32_t toupper(char32_t cp) noexcept { return (cp >= 97 && cp <= 122) ? (cp ^ 0b100000) : cp; }
	constexpr char32_t tolower(char32_t cp) noexcept { return (cp >= 65 && cp <= 90) ? (cp | 0b100000) : cp; }

	/// @}

	/// Convert a number between 0 and 9 to its ASCII representation (only gives meaningful results with arguments between 0 and 9)
	constexpr char32_t number_to_digit(int v) noexcept { return char32_t(v) + 48; }
	/// Convert a number between 0 and 15 to its ASCII representation (only gives meaningful results with arguments between 0 and 15)
	constexpr char32_t number_to_xdigit(int v) noexcept { return (v > 9) ? (char32_t(v - 10) + 65) : (char32_t(v) + 48); }

	/// Convert an ASCII digit character to its numerical value (only gives meaningful results with valid digit arguments)
	constexpr int digit_to_number(char32_t cp) noexcept { return int(cp - 48); }
	/// Convert an ASCII xdigit to its numerical value (only gives meaningful results with valid xdigit arguments)
	//constexpr int xdigit_to_number(char32_t cp) noexcept { return (cp >= 97 && cp <= 102) ? int(cp - 97) : int((cp >= 65 && cp <= 70) ? (cp - 55) : (cp - 48)); }
	constexpr int xdigit_to_number(char32_t cp) noexcept { return isdigit(cp) ? int(cp - 48) : ((int(cp) & ~0b100000) - 55); }
}
/// @}


/// \name Trimming Functions
/// Functions that trim strings and string_views.
/// @{

inline string_view trimmed_whitespace_right(string_view str) noexcept { return string_view(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ascii::isspace).base()); }
inline string_view trimmed_whitespace_left(string_view str) noexcept { return string_view(std::find_if_not(str.begin(), str.end(), ascii::isspace), str.end()); }
inline string_view trimmed_whitespace(string_view str) noexcept { return trimmed_whitespace_left(trimmed_whitespace_right(str)); }
inline string_view trimmed_until(string_view str, char chr) noexcept { return string_view(std::find(str.begin(), str.end(), chr), str.end()); }
inline string_view trimmed(string_view str, char chr) noexcept { return string_view(std::find_if_not(str.begin(), str.end(), [chr](char c) { return c == chr; }), str.end()); }

inline std::string trimmed_whitespace_right(std::string str) noexcept { str.erase(std::find_if_not(str.rbegin(), str.rend(), ascii::isspace).base(), str.end()); return str; }
inline std::string trimmed_whitespace_left(std::string str) noexcept { str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), ascii::isspace)); return str; }
inline std::string trimmed_whitespace(std::string str) noexcept { return trimmed_whitespace_left(trimmed_whitespace_right(std::move(str))); }
inline std::string trimmed_until(std::string str, char chr) noexcept { str.erase(str.begin(), std::find(str.begin(), str.end(), chr)); return str; }
inline std::string trimmed(std::string str, char chr) noexcept { str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), [chr](char c) { return c == chr; })); return str; }
template <typename FUNC>
inline string_view trimmed_while(string_view str, FUNC&& func) noexcept { return ::ghassanpl::string_ops::string_view(std::find_if_not(str.begin(), str.end(), std::forward<FUNC>(func)), str.end()); }

inline void trim_whitespace_right(string_view& str) noexcept { str = string_view(str.begin(), std::find_if_not(str.rbegin(), str.rend(), ascii::isspace).base()); }
inline void trim_whitespace_left(string_view& str) noexcept { str = string_view(std::find_if_not(str.begin(), str.end(), ascii::isspace), str.end()); }
inline void trim_whitespace(string_view& str) noexcept { trim_whitespace_left(str); trim_whitespace_right(str); }
inline void trim_until(string_view& str, char chr) noexcept { str = trimmed_until(str, chr); }
inline void trim(string_view& str, char chr) noexcept { str = trimmed(str, chr); }
template <typename FUNC>
inline void trim_while(string_view& str, FUNC&& func) noexcept { str = trimmed_while(str, std::forward<FUNC>(func)); }

/// @}

/// \name Consume Functions
/// Functions that "consume" parts of a `string_view` (that is, remove a section from the beginning or end if the conditions apply).
/// Most of the functions return the consumed part, or 'true/false' if the part to be consumed is given explicitly.
/// These functions do nothing (or the maximum safe amount) if there is nothing appropriate available to consume.
/// @{

/// Consumes and returns the first character in the str, or \0 if no more characters.
inline char consume(string_view& str)
{
	if (str.empty())
		return {};
	const auto result = str[0];
	str.remove_prefix(1);
	return result;
}

/// Consumes the character `val` if it's at the beginning of `str`
/// \returns whether it actually consumed
inline bool consume(string_view& str, char val)
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
inline bool consume(string_view& str, string_view val)
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
inline char consume_any(string_view& str, string_view chars)
{
	if (!str.empty() && chars.contains(str[0]))
	{
		const auto result = str[0];
		str.remove_prefix(1);
		return result;
	}
	return 0;
}

/// Consumes a run of any of the characters in 'chars' at the beginning of str
/// \returns the consumed character, or \0 if none found
inline string_view consume_while_any(string_view& str, string_view chars)
{
	const auto start = str.begin();
	while (!str.empty() && chars.contains(str[0]))
		str.remove_prefix(1);
	return string_view{ start, str.begin() };
}

/// Consumes a character from the beginning of `str` if it matches `pred(str[0])`.
/// \returns the matched character, or \0 if no match
template <typename PRED>
inline char consume(string_view& str, PRED&& pred)
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
inline char consume_or(string_view& str, char or_else)
{
	if (str.empty())
		return or_else;
	const auto result = str[0];
	str.remove_prefix(1);
	return result;
}

/// Consumes the last character from `str` if it matches `val`.
/// \returns whether it consumed
/// \see consume(string_view&, char)
inline bool consume_at_end(string_view& str, char val)
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
/// \see consume(string_view&, char)
inline bool consume_at_end(string_view& str, string_view val)
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
inline string_view consume_while(string_view& str, FUNC&& pred)
{
	const auto start = str.begin();
	while (!str.empty() && pred(str[0]))
		str.remove_prefix(1);
	return string_view(start, str.begin());
}

/// Consumes characters from the beginning of `str` while they are equal to `c`.
/// \returns the consumed prefix as a string_view
inline string_view consume_while(string_view& str, char c)
{
	const auto start = str.begin();
	while (str.starts_with(c))
		str.remove_prefix(1);
	return string_view(start, str.begin());
}

/// Consumes characters from the beginning of `str` until one matches `pred(str[0])`, exclusive.
/// \returns the consumed prefix as a string_view
template <typename FUNC>
inline string_view consume_until(string_view& str, FUNC&& pred)
{
	const auto start = str.begin();
	while (!str.empty() && !pred(str[0]))
		str.remove_prefix(1);
	return string_view(start, str.begin());
}

/// Consumes characters from the beginning of `str` until one is equal to `c`, exclusive.
/// \returns the consumed prefix as a string_view
inline string_view consume_until(string_view& str, char c)
{
	const auto start = str.begin();
	while (!str.empty() && str[0] != c)
		str.remove_prefix(1);
	return string_view(start, str.begin());
}

/// Consumes characters from the beginning of `str` until the string starts with `end`, exclusive.
/// \returns the consumed prefix as a string_view
inline string_view consume_until(string_view& str, string_view end)
{
	const auto it = std::search(str.begin(), str.end(), end.begin(), end.end());
	const auto result = string_view(str.begin(), it);
	str = { it, str.end() };
	return result;
}

/// Consumes characters from the beginning of `str` until one is equal to `c`, **inclusive**.
/// \returns the consumed prefix as a string_view
inline string_view consume_until_delim(string_view& str, char c)
{
	/// TODO: Should this return a sv including the delimiter?

	const auto start = str.begin();
	while (!str.empty() && str[0] != c)
		str.remove_prefix(1);
	(void)consume(str, c);
	return string_view(start, str.begin());
}

/// Consumes at most `n` characters from the beginning of `str`.
/// \returns the consumed prefix as a string_view
inline string_view consume_n(string_view& str, size_t n)
{
	n = std::min(str.size(), n);
	auto result = str.substr(0, n);
	str.remove_prefix(n);
	return result;
}

/// Consumes at most `n` characters from the beginning of `str` that match `pred(str[0])`.
/// \returns the consumed prefix as a string_view
template <typename FUNC>
inline string_view consume_n(string_view& str, size_t n, FUNC&& pred)
{
	n = std::min(str.size(), n);
	const auto start = str.begin();
	while (n-- && !str.empty() && pred(str[0]))
		str.remove_prefix(1);
	return string_view(start, str.begin());
}


/// \name Split Functions
/// Functions that split strings into multiple parts, each delimited with some sort of delimiter.
/// @{

/// Performs a basic "split" operation, calling `func` for each part of `source` delimited by `delim`.
/// If no delimiters are found, calls `func` for entire string.
/// \param func must be invocable as `func(string_view, bool)`; the second argument specifies if the given string is the final part of the split
template <typename FUNC>
void split(string_view source, char delim, FUNC&& func)
{
	size_t next = 0;
	while ((next = source.find_first_of(delim)) != std::string::npos)
	{
		func(source.substr(0, next), false);
		source.remove_prefix(next + 1);
	}
	func(source, true);
}

/// Performs a basic "split" operation, calling `func` for each part of `source` delimited by `delim`.
/// If no delimiters are found, calls `func` for entire string.
/// \param func must be invocable as `func(string_view, bool)`; the second argument specifies if the given string is the final part of the split
template <typename FUNC>
void split(string_view source, string_view delim, FUNC&& func)
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

/// Splits `src` once on `delim`
/// \returns a pair of string_views: the left and right parts of `src` split the first instance of `delim`; if no delim is found, returns `{ src, {} }`
inline std::pair<string_view, string_view> single_split(string_view src, char delim) noexcept
{
	size_t split_at = src.find_first_of(delim);
	if (split_at == std::string::npos)
		return { src, {} };
	return { src.substr(0, split_at), src.substr(split_at + 1) };
}

/// Splits `src` once on `delim`
/// \returns whether delim was found
/// \param first will be filled with the left part of the string, if delim is found
/// \param second will be filled with the right part of the string, if delim is found
inline bool single_split(string_view src, char delim, string_view* first, string_view* second) noexcept
{
	size_t split_at = src.find_first_of(delim);
	if (split_at == std::string::npos)
		return false;
	if (first) *first = src.substr(0, split_at);
	if (second) *second = src.substr(split_at + 1);
	return true;
}

/// @}



/// \name Join Functions
/// Functions that join a range of formattable elements into a single string
/// \note Formatting is done using stream operators (`operator<<`).
/// \todo Use \c Stringification instead
/// @{

/// Returns a string that is created by joining together string representation of the elements in the `source` range.
template <typename T>
std::string join(T&& source)
{
	std::stringstream strm{};
	for (auto&& p : std::forward<T>(source))
		strm << p;
	return strm.str();
}

/// Returns a string that is created by joining together string representation of the elements in the `source` range, separated by `delim`; `delim` is only added between elements.
template <typename T, typename DELIM>
std::string join(T&& source, DELIM const& delim)
{
	std::stringstream strm{};
	bool first = true;
	for (auto&& p : std::forward<T>(source))
	{
		if (!first) strm << delim;
		strm << p;
		first = false;
	}
	return strm.str();
}

/// Same as \c join(T&& source, DELIM&& delim)
/// except each element is transformed by `transform_func` before being stringified and added to the result.
/// \param transform_func must be invocable as `transform_fun(el)` for each element in the `source` range
template <typename T, typename FUNC, typename DELIM>
std::string join(T&& source, DELIM const& delim, FUNC&& transform_func)
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

namespace detail
{
	struct from_chars_result
	{
		const char* ptr = nullptr;
		bool failed = false;
	};

	template <class T>
	from_chars_result integer_from_chars(const char* const first, const char* const last, T& out_value, const int base) noexcept
	{
		assert(base >= 2 && base <= 36);

		bool minus_sign = false;
		const char* next = first;

		if (std::is_signed<T>::value && next != last && *next == '-')
		{
			minus_sign = true;
			++next;
		}

		using unsigned_t = std::make_unsigned<T>::type;

		const unsigned_t uint_max_val = static_cast<unsigned_t>(-1);
		const unsigned_t int_max_val = static_cast<unsigned_t>(uint_max_val >> 1);
		const unsigned_t abs_int_min_val = static_cast<unsigned_t>(int_max_val + 1);

		unsigned_t risky_val{};
		unsigned_t max_digit{};

		if (std::is_signed<T>::value)
		{
			if (minus_sign)
			{
				risky_val = static_cast<unsigned_t>(abs_int_min_val / base);
				max_digit = static_cast<unsigned_t>(abs_int_min_val % base);
			}
			else
			{
				risky_val = static_cast<unsigned_t>(int_max_val / base);
				max_digit = static_cast<unsigned_t>(int_max_val % base);
			}
		}
		else
		{
			risky_val = static_cast<unsigned_t>(uint_max_val / base);
			max_digit = static_cast<unsigned_t>(uint_max_val % base);
		}

		unsigned_t value{};

		bool overflowed = false;

		for (; next != last; ++next)
		{
			const int digit = ascii::digit_to_number(static_cast<char32_t>(*next));

			if (digit >= base)
				break;

			if (value < risky_val || (value == risky_val && static_cast<unsigned_t>(digit) <= max_digit))
			{
				value = static_cast<unsigned_t>(value * base + digit);
			}
			else
			{
				overflowed = true;
			}
		}

		if (next - first == static_cast<ptrdiff_t>(minus_sign))
			return { first, true };

		if (overflowed)
			return { next, true };

		if (std::is_signed<T>::value && minus_sign)
			value = static_cast<unsigned_t>(0 - value);

		out_value = static_cast<T>(value);

		return { next, false };
	}

	template <typename T>
	T string_to_number(string_view str, size_t* idx = nullptr, int base = 10) noexcept
	{
		T value{};
		const auto res = integer_from_chars(str.data(), str.data_end(), value, base);
		if (idx && !res.failed)
			*idx = size_t(res.ptr - str.data());
		return value;
	}
	
	template <typename T>
	inline T consume_num(string_view& str, int base = 10) noexcept
	{
		size_t idx = 0;
		const T result = detail::string_to_number<T>(str, &idx, base);
		str.remove_prefix(idx);
		return result;
	}
}

/// \name sto* replacements
/// Functions equivalent to `std::stoi`, `std::stoull`, etc that take `string_view` as its first argument
/// @{
inline int stoi(string_view str, size_t* idx = nullptr, int base = 10) noexcept { return detail::string_to_number<int>(str, idx, base); }
inline long stol(string_view str, size_t* idx = nullptr, int base = 10) noexcept { return detail::string_to_number<long>(str, idx, base); }
inline long long stoll(string_view str, size_t* idx = nullptr, int base = 10) noexcept { return detail::string_to_number<long long>(str, idx, base); }
inline unsigned long stoul(string_view str, size_t* idx = nullptr, int base = 10) noexcept { return detail::string_to_number<unsigned long>(str, idx, base); }
inline unsigned long long stoull(string_view str, size_t* idx = nullptr, int base = 10) noexcept { return detail::string_to_number<unsigned long long>(str, idx, base); }

inline int consume_int(string_view& str, int base = 10) noexcept { return detail::consume_num<int>(str, base); }
inline long consume_long(string_view& str, int base = 10) noexcept { return detail::consume_num<long>(str, base); }
inline long long consume_long_long(string_view& str, int base = 10) noexcept { return detail::consume_num<long long>(str, base); }
inline unsigned long consume_unsigned_long(string_view& str, int base = 10) noexcept { return detail::consume_num<unsigned long>(str, base); }
inline unsigned long long consume_usigned_long_long(string_view& str, int base = 10) noexcept { return detail::consume_num<unsigned long long>(str, base); }

template <typename T>
bool consume_num(string_view& str, T& out_val, int base = 10) noexcept
{
	const auto res = detail::integer_from_chars(str.data(), str.data_end(), value, base);
	return !res.failed;
}
/// @}
