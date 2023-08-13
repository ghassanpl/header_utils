#pragma once

#include <cstddef>
#include <span>
#include <string_view>

namespace ghassanpl
{

	/// Represents any trivially copyable type that is trivially castable to an internal object representation (so a char range)
	template <typename T>
	concept bytelike = (alignof(T) == alignof(std::byte) && sizeof(T) == sizeof(std::byte) && std::is_trivial_v<T>);

	template <typename T>
	concept bytelike_range = std::ranges::range<T> && bytelike<std::ranges::range_value_t<T>>;

	/// Converts a span of trivial values to a span of \c bytelike s
	template <bytelike TO, typename FROM>
	requires std::is_trivially_copyable_v<FROM>
	constexpr std::span<TO const> as_bytelikes(std::span<FROM const> bytes) noexcept
	{
		return { reinterpret_cast<TO const*>(bytes.data()), bytes.size() * sizeof(FROM) };
	}

	/// Returns a span of \c bytelike s that represents the internal object representation of the argument
	template <bytelike TO, typename T>
	requires std::is_trivially_copyable_v<T>
	constexpr std::span<TO const> as_bytelikes(T const& pod) noexcept
	{
		return { reinterpret_cast<TO const*>(std::addressof(pod)), sizeof(pod) };
	}
	
	template <bytelike FROM> constexpr auto to_u8(FROM byte) noexcept { return std::bit_cast<uint8_t>(byte); }
	template <bytelike FROM> constexpr auto to_char(FROM byte) noexcept { return std::bit_cast<char>(byte); }
	template <bytelike FROM> constexpr auto to_byte(FROM byte) noexcept { return std::bit_cast<std::byte>(byte); }
	template <bytelike FROM> constexpr auto to_uchar(FROM byte) noexcept { return std::bit_cast<unsigned char>(byte); }
	template <bytelike FROM> constexpr auto to_char8(FROM byte) noexcept { return std::bit_cast<char8_t>(byte); }

	template <bytelike FROM> constexpr auto as_chars(std::span<FROM const> bytes) noexcept { return as_bytelikes<char>(bytes); }
	template <bytelike FROM> constexpr auto as_bytes(std::span<FROM const> bytes) noexcept { return as_bytelikes<std::byte>(bytes); }
	template <bytelike FROM> constexpr auto as_uchars(std::span<FROM const> bytes) noexcept { return as_bytelikes<unsigned char>(bytes); }
	template <bytelike FROM> constexpr auto as_u8s(std::span<FROM const> bytes) noexcept { return as_bytelikes<uint8_t>(bytes); }
	template <bytelike FROM> constexpr auto as_char8s(std::span<FROM const> bytes) noexcept { return as_bytelikes<char8_t>(bytes); }

	constexpr auto as_chars(std::string_view bytes) noexcept { return as_bytelikes<char>(std::span{ bytes }); }
	constexpr auto as_bytes(std::string_view bytes) noexcept { return as_bytelikes<std::byte>(std::span{ bytes }); }
	constexpr auto as_uchars(std::string_view bytes) noexcept { return as_bytelikes<unsigned char>(std::span{ bytes }); }
	constexpr auto as_u8s(std::string_view bytes) noexcept { return as_bytelikes<uint8_t>(std::span{ bytes }); }
	constexpr auto as_char8s(std::string_view bytes) noexcept { return as_bytelikes<char8_t>(std::span{ bytes }); }

	template <typename T> requires std::is_trivial_v<T> constexpr auto as_chars(T const& data) noexcept { return as_bytelikes<char>(data); }
	template <typename T> requires std::is_trivial_v<T> constexpr auto as_bytes(T const& data) noexcept { return as_bytelikes<std::byte>(data); }
	template <typename T> requires std::is_trivial_v<T> constexpr auto as_uchars(T const& data) noexcept { return as_bytelikes<unsigned char>(data); }
	template <typename T> requires std::is_trivial_v<T> constexpr auto as_u8s(T const& data) noexcept { return as_bytelikes<uint8_t>(data); }
	template <typename T> requires std::is_trivial_v<T> constexpr auto as_char8s(T const& data) noexcept { return as_bytelikes<char8_t>(data); }

#if 0
	/// Returns a `char` span that represents the internal object representation of the argument
	template <typename T>
	requires std::is_trivially_copyable_v<T>
	std::span<char const> as_uchars(T const& pod)
	{
		return { reinterpret_cast<unsigned char const*>(&pod), sizeof(pod) };
	}


	/// Converts a span of bytelikes to a span of `std::byte`s
	template <bytelike T>
	std::span<std::byte const> as_bytes(std::span<T const> bytes)
	{
		return { reinterpret_cast<std::byte const*>(bytes.data()), bytes.size() };
	}

	/// Returns a `std::byte` span that represents the internal object representation of the argument
	template <typename T>
	requires std::is_trivially_copyable_v<T>
	std::span<std::byte const> as_bytes(T const& pod)
	{
		return { reinterpret_cast<std::byte const*>(&pod), sizeof(pod) };
	}

	/// Returns a span that represents the internal object representation of the argument, of any \ref bytelike type
	template <bytelike B, typename T>
	requires std::is_trivially_copyable_v<T>
	std::span<B const> as_bytes(T const& pod)
	{
		return { reinterpret_cast<B const*>(&pod), sizeof(pod) };
	}

	/// Converts a span of \ref bytelike s to a span of `char`s
	template <bytelike T>
	std::span<char const> as_chars(std::span<T const> bytes)
	{
		return { reinterpret_cast<char const*>(bytes.data()), bytes.size() };
	}

	/// Returns a `char` span that represents the internal object representation of the argument
	template <typename T>
	requires std::is_trivially_copyable_v<T>
	std::span<char const> as_chars(T const& pod)
	{
		return { reinterpret_cast<char const*>(&pod), sizeof(pod) };
	}

	/// Converts a span of \ref bytelike s to a span of `char`s
	template <bytelike T>
	std::span<char const> as_uchars(std::span<T const> bytes)
	{
		return { reinterpret_cast<unsigned char const*>(bytes.data()), bytes.size() };
	}

	/// Returns a `char` span that represents the internal object representation of the argument
	template <typename T>
	requires std::is_trivially_copyable_v<T>
	std::span<char const> as_uchars(T const& pod)
	{
		return { reinterpret_cast<unsigned char const*>(&pod), sizeof(pod) };
	}

	/// Converts a span of \ref bytelike s to a span of `char`s
	template <bytelike T>
	std::span<char const> as_u8s(std::span<T const> bytes)
	{
		return { reinterpret_cast<unsigned char const*>(bytes.data()), bytes.size() };
	}

	/// Returns a `char` span that represents the internal object representation of the argument
	template <typename T>
		requires std::is_trivially_copyable_v<T>
	std::span<char const> as_u8s(T const& pod)
	{
		return { reinterpret_cast<unsigned char const*>(&pod), sizeof(pod) };
	}
#endif
}