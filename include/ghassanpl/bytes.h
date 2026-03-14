/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string_view>
#include <array>
#include <bit>
#include <memory>
#include <stdexcept>
#include <span>

namespace ghassanpl
{
	template<typename S, typename D>
	using copy_const_t = std::conditional_t<std::is_const_v<S>, std::add_const_t<D>, std::remove_const_t<D>>;

	/// \defgroup Bytes Bytes
	/// A fairly UB-free set of functions to operate on and convert between arrays of bytelike types
	/// @{

	/// Represents any trivially copyable type that is trivially castable to an internal object representation (so a char range)
	template <typename T>
	concept bytelike = (alignof(T) == alignof(std::byte) && sizeof(T) == sizeof(std::byte) && std::is_trivial_v<T>);

	static_assert(bytelike<char>);
	static_assert(bytelike<unsigned char>);
	static_assert(bytelike<uint8_t>);
	static_assert(bytelike<char8_t>);

	/// A range of bytelikes
	template <typename T>
	concept bytelike_range = std::ranges::range<T> && bytelike<std::ranges::range_value_t<T>>;

	/// Converts a span of trivial values to a span of \c bytelike s
	template <bytelike TO, typename FROM, size_t N = std::dynamic_extent>
	requires std::is_trivially_copyable_v<FROM>
	[[nodiscard]] std::span<TO> as_bytelikes(std::span<FROM, N> elements) noexcept
	{
		return { reinterpret_cast<TO*>(elements.data()), elements.size() * sizeof(FROM) };
	}

	/// Returns an object whose internal representation is initialized from the argument
	template <typename TO, bytelike FROM>
	requires std::is_trivially_copyable_v<TO>
	[[nodiscard]] TO from_bytelikes(std::span<FROM const> from)
	{
		/// TODO: Use `expected` result instead of exceptions?
		if (from.size() < sizeof(TO))
			throw std::invalid_argument("from span must be at least the size of the object");
		TO result;
		std::memcpy(std::addressof(result), from.data(), sizeof(TO));
		return result;
	}

	/// Returns an object whose internal representation is initialized from the argument
	template <typename TO, bytelike FROM>
	requires std::is_trivially_copyable_v<TO>
	[[nodiscard]] std::span<TO> span_from_bytelikes(std::span<FROM> from)
	{
		const auto size = from.size() / sizeof(TO);
		return std::span<TO>{ reinterpret_cast<TO*>(from.data()), size };
	}


	/// Returns a span of \c bytelike s that represents the internal object representation of the argument
	template <bytelike TO, typename T>
	requires std::is_trivially_copyable_v<T>
	[[nodiscard]] auto as_bytelikes(T& pod) noexcept
	{
		if constexpr (std::is_const_v<T>)
			return std::span{ reinterpret_cast<TO const*>(std::addressof(pod)), sizeof(pod) };
		else
			return std::span{ reinterpret_cast<TO*>(std::addressof(pod)), sizeof(pod) };
	}
	
	/// Bit-casts the given bytelike to a `uint8_t`
	template <bytelike FROM> [[nodiscard]] constexpr auto to_u8(FROM byte) noexcept { return std::bit_cast<uint8_t>(byte); }
	/// Bit-casts the given bytelike to a `char`
	template <bytelike FROM> [[nodiscard]] constexpr auto to_char(FROM byte) noexcept { return std::bit_cast<char>(byte); }
	/// Bit-casts the given bytelike to a `std::byte`
	template <bytelike FROM> [[nodiscard]] constexpr auto to_byte(FROM byte) noexcept { return std::bit_cast<std::byte>(byte); }
	/// Bit-casts the given bytelike to a `unsigned char`
	template <bytelike FROM> [[nodiscard]] constexpr auto to_uchar(FROM byte) noexcept { return std::bit_cast<unsigned char>(byte); }
	/// Bit-casts the given bytelike to a `char8_t`
	template <bytelike FROM> [[nodiscard]] constexpr auto to_char8(FROM byte) noexcept { return std::bit_cast<char8_t>(byte); }

	/// Returns a span of `char`s that represents the bytelikes in `bytes`
	template <bytelike FROM, size_t N = std::dynamic_extent> 
	[[nodiscard]] auto as_chars(std::span<FROM, N> bytes) noexcept { return as_bytelikes<copy_const_t<FROM, char>>(bytes); }
	/// Returns a span of `std::byte`s that represents the bytelikes in `bytes`
	template <bytelike FROM, size_t N = std::dynamic_extent> 
	[[nodiscard]] auto as_bytes(std::span<FROM, N> bytes) noexcept { return as_bytelikes<copy_const_t<FROM, std::byte>>(bytes); }
	/// Returns a span of `unsigned char`s that represents the bytelikes in `bytes`
	template <bytelike FROM, size_t N = std::dynamic_extent> 
	[[nodiscard]] auto as_uchars(std::span<FROM, N> bytes) noexcept { return as_bytelikes<copy_const_t<FROM, unsigned char>>(bytes); }
	/// Returns a span of `uint8_t`s that represents the bytelikes in `bytes`
	template <bytelike FROM, size_t N = std::dynamic_extent> 
	[[nodiscard]] auto as_u8s(std::span<FROM, N> bytes) noexcept { return as_bytelikes<copy_const_t<FROM, uint8_t>>(bytes); }
	/// Returns a span of `char8_t`s that represents the bytelikes in `bytes`
	template <bytelike FROM, size_t N = std::dynamic_extent> 
	[[nodiscard]] auto as_char8s(std::span<FROM, N> bytes) noexcept { return as_bytelikes<copy_const_t<FROM, char8_t>>(bytes); }

	/// Returns a span of `char`s that represents the internal object representation of the `data` object
	template <typename T> requires std::is_trivial_v<T> [[nodiscard]] auto as_chars(T const& data) noexcept { return as_bytelikes<char>(data); }
	/// Returns a span of `std::byte`s that represents the internal object representation of the `data` object
	template <typename T> requires std::is_trivial_v<T> [[nodiscard]] auto as_bytes(T const& data) noexcept { return as_bytelikes<std::byte>(data); }
	/// Returns a span of `unsigned char`s that represents the internal object representation of the `data` object
	template <typename T> requires std::is_trivial_v<T> [[nodiscard]] auto as_uchars(T const& data) noexcept { return as_bytelikes<unsigned char>(data); }
	/// Returns a span of `uint8_t`s that represents the internal object representation of the `data` object
	template <typename T> requires std::is_trivial_v<T> [[nodiscard]] auto as_u8s(T const& data) noexcept { return as_bytelikes<uint8_t>(data); }
	/// Returns a span of `char8_t`s that represents the internal object representation of the `data` object
	template <typename T> requires std::is_trivial_v<T> [[nodiscard]] auto as_char8s(T const& data) noexcept { return as_bytelikes<char8_t>(data); }

	/// Returns the value of the nth bit in the range of bytelikes
	template <bytelike B, size_t N = std::dynamic_extent>
	[[nodiscard]] bool nth_bit(std::span<B const, N> range, size_t n) noexcept
	{
		if (n >= range.size() * CHAR_BIT)
			return false;
		const auto u8range = as_u8s(range);
		const auto byte = u8range[n / CHAR_BIT];
		return byte & (1 << (n % CHAR_BIT));
	}

	/// Sets the value of the nth bit in the range of bytelikes
	template <bytelike B, size_t N = std::dynamic_extent>
	void set_nth_bit(std::span<B, N> range, size_t n, bool value) noexcept
	{
		if (n >= range.size() * CHAR_BIT)
			return;
		auto u8range = as_u8s(range);
		auto byte = u8range[n / CHAR_BIT];
		if (value)
			byte |= (1 << (n % CHAR_BIT));
		else
			byte &= ~(1 << (n % CHAR_BIT));
	}

	/// Returns the value of the nth bit in the internal object representation of the `pod` object
	template <typename T>
	requires std::is_trivially_copyable_v<T>
	[[nodiscard]] bool nth_bit(T const& pod, size_t n) noexcept
	{
		return nth_bit(as_u8s(pod), n);
	}

	/// Sets the value of the nth bit in the internal object representation of the `pod` object
	template <typename T>
	requires std::is_trivially_copyable_v<T>
	void set_nth_bit(T& pod, size_t n, bool value) noexcept
	{
		set_nth_bit(as_u8s(pod), n, value);
	}

	/// Returns the value of the Nth bit in the range of bytelikes
	template <size_t N, bytelike B, size_t SN = std::dynamic_extent>
	[[nodiscard]] bool nth_bit(std::span<B const, SN> range) noexcept
	{
		if (N >= range.size() * CHAR_BIT)
			return false;
		const auto u8range = as_u8s(range);
		const auto byte = u8range[N / CHAR_BIT];
		return byte & (1 << (N % CHAR_BIT));
	}

	/// Sets the value of the Nth bit in the range of bytelikes
	template <size_t N, bytelike B, size_t SN = std::dynamic_extent>
	void set_nth_bit(std::span<B, SN> range, bool value) noexcept
	{
		if (N >= range.size() * CHAR_BIT)
			return;
		auto u8range = as_u8s(range);
		auto byte = u8range[N / CHAR_BIT];
		if (value)
			byte |= (1 << (N % CHAR_BIT));
		else
			byte &= ~(1 << (N % CHAR_BIT));
	}

	/// Returns the value of the Nth bit in the internal object representation of the `pod` object
	template <size_t N, typename T>
	requires std::is_trivially_copyable_v<T>
	[[nodiscard]] bool nth_bit(T const& pod) noexcept
	{
		return nth_bit<N>(as_u8s(pod));
	}

	/// Sets the value of the Nth bit in the internal object representation of the `pod` object
	template <size_t N, typename T>
	requires std::is_trivially_copyable_v<T>
	void set_nth_bit(T& pod, bool value) noexcept
	{
		set_nth_bit<N>(as_u8s(pod), value);
	}

	/// A constexpr function that converts an integral value to its constituent bytelikes
	/// TODO: This is NOT like a reinterpret_/bit_cast to u8s, because it's not endian-aware
	template <bytelike B, std::integral T>
	[[nodiscard]] constexpr auto to_bytelike_array(T value_)
	{
		auto value = std::bit_cast<std::make_unsigned_t<T>>(value_);
		std::array<B, sizeof(T)> result;
		for (size_t i = 0; i < sizeof(T); ++i)
		{
			result[i] = static_cast<B>(value & 0xFF);
			value >>= 8;
		}
		return result;
	}

	template <std::integral T> constexpr auto to_u8_array(T value) { return to_bytelike_array<uint8_t>(value); }

	template <bytelike T>
	struct align_front_to_result
	{
		std::span<T> prefix;
		std::span<T> aligned;
	};

	/// Splits the argument span into two spans, the first one being the prefix, the second one having its data pointer aligned to the given alignment.
	template <size_t ALIGN, bytelike T, size_t N = std::dynamic_extent>
	[[nodiscard]] auto align_front_to(std::span<T, N> bytes) noexcept -> align_front_to_result<T>
	{
		static_assert(ALIGN >= 1, "Alignment must be greater or equal to 1");
		if constexpr (ALIGN == 1)
		{
			return bytes;
		}
		else
		{
			void* ptr = const_cast<std::remove_const_t<T>*>(bytes.data());
			size_t size = bytes.size();
			ptr = std::align(ALIGN, ALIGN, ptr, size);
			if (!ptr)
				return { bytes, {} };
			
			return { { bytes.data(), bytes.size() - size}, { const_cast<T*>(reinterpret_cast<std::remove_const_t<T>*>(ptr)), size }};
		}
	}

	template <bytelike T>
	struct align_back_to_result
	{
		std::span<T> aligned;
		std::span<T> suffix;
	};

	/// Splits the argument span into two spans, the first one having its size aligned to the given alignment, the second one being the suffix.
	/// Assumes/expects bytes.data() is aligned to the specified alignment.
	template <size_t ALIGN, bytelike T, size_t N = std::dynamic_extent>
	[[nodiscard]] constexpr auto align_back_to(std::span<T, N> bytes) noexcept -> align_back_to_result<T>
	{
		static_assert(ALIGN >= 1, "Alignment must be greater or equal to 1");
		if constexpr (ALIGN == 1)
		{
			return bytes;
		}
		else
		{
			const auto size = bytes.size();

			if (size < ALIGN)
				return { {}, bytes };

			const auto aligned_size = size - (size % ALIGN);
			return { bytes.subspan(0, aligned_size), bytes.subspan(aligned_size, size % ALIGN) };
		}
	}

	template <bytelike T, typename MIDDLE = T>
	struct align_to_result
	{
		std::span<T> prefix;
		std::span<MIDDLE> aligned;
		std::span<T> suffix;
	};

	/// Splits the argument span into three spans, the first one being the prefix, the second one having its data and size aligned to the given alignment, 
	/// the third one being the suffix.
	template <size_t ALIGN, bytelike T, size_t N = std::dynamic_extent>
	[[nodiscard]] auto align_to(std::span<T, N> bytes) noexcept -> align_to_result<T>
	{
		static_assert(ALIGN >= 1, "Alignment must be greater or equal to 1");
		if constexpr (ALIGN == 1)
		{
			return bytes;
		}
		else
		{
			void* ptr = const_cast<std::remove_const_t<T>*>(bytes.data());
			size_t size = bytes.size();
			ptr = std::align(ALIGN, ALIGN, ptr, size);
			if (!ptr)
				return { bytes, {}, {} };

			const auto aligned_size = size - (size % ALIGN);
			return { 
				{ bytes.data(), bytes.size() - size}, 
				{ const_cast<T*>(reinterpret_cast<std::remove_const_t<T>*>(ptr)), aligned_size }, 
				{ const_cast<T*>(reinterpret_cast<std::remove_const_t<T>*>(ptr)) + aligned_size, size % ALIGN }
			};
		}
	}

	/// Splits the argument span into three spans, the first one being the prefix, the second one capable of storing one ore more objects of type `TO`
	/// with the proper alignment, and the third one being the suffix.
	template <typename TO, bytelike B, size_t N = std::dynamic_extent>
	[[nodiscard]] auto aligned_span_cast(std::span<B, N> bytes) noexcept -> align_to_result<B, TO>
	{
		const auto [prefix, aligned_original, suffix] = align_to<std::max(alignof(TO), sizeof(TO))>(bytes);
		const std::span<TO> aligned { 
			reinterpret_cast<TO*>(aligned_original.data()), 
			reinterpret_cast<TO*>(aligned_original.data() + aligned_original.size())
		};
		return { prefix, aligned, suffix };
	}

	/// Does the same as `aligned_span_cast` but then default-constructs the elements of the of middle span.
	template <typename TO, bytelike B, size_t N = std::dynamic_extent>
	[[nodiscard]] auto aligned_span_cast_and_construct(std::span<B, N> bytes) noexcept -> align_to_result<B, TO>
	{
		const auto result = aligned_span_cast<TO>(bytes);
		std::uninitialized_default_construct_n(result.aligned.data(), result.aligned.size());
		return result;
	}

	/// @}

	/// TODO: Maybe a safe_span_cast that returns an `expected`?
}