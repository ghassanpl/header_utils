/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#if defined(__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L
#include <bit>
#endif
#if defined(__cpp_lib_forward_like)
#include <utility>
#endif
#include <memory>

#include "min-cpp-version/cpp20.h"

namespace ghassanpl
{
	template<class T, class U>
	constexpr std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<U>&& r) noexcept
	{
		if (auto p = dynamic_cast<T*>(r.get()))
			return (void)r.release(), std::unique_ptr<T>(p);
		else
			return nullptr;
	}

	template<class T, class D, class U>
	constexpr std::unique_ptr<T, D> dynamic_pointer_cast(std::unique_ptr<U, D>&& r) noexcept
	{
		if (auto p = dynamic_cast<std::unique_ptr<T, D>::pointer>(r.get()))
			return (void)r.release(), std::unique_ptr<T, D>(p, std::forward<D>(r.get_deleter()));
		else if constexpr (!std::is_pointer_v<D> && std::is_default_constructible_v<D>)
			return nullptr;
		else if constexpr (std::is_copy_constructible_v<D>)
			return std::unique_ptr<T, D>(nullptr, r.get_deleter());
	}

	template <class T, class... TYPES>
	constexpr inline bool is_any_of_v = std::disjunction_v<std::is_same<T, TYPES>...>;

#if defined(__cpp_concepts)
	namespace detail
	{
		template <class _Ty>
		using with_reference = _Ty&;

		template <class T>
		concept can_reference = requires { typename with_reference<T>; };
	}

	template <class T>
	concept dereferenceable = requires(T & t) {
		{ *t } -> detail::can_reference;
	};
#endif

#if defined(__cpp_lib_forward_like)
	using std::forward_like;
#else
	template <class Ty, class Uty>
	[[nodiscard]] constexpr auto&& forward_like(Uty&& Ux) noexcept
	{
		static_assert(detail::can_reference<Ty>, "forward_like's first template argument must be a referenceable type.");

		using UnrefT = std::remove_reference_t<Ty>;
		using UnrefU = std::remove_reference_t<Uty>;
		if constexpr (std::is_const_v<UnrefT>) {
			if constexpr (std::is_lvalue_reference_v<Ty>) {
				return static_cast<const UnrefU&>(Ux);
			}
			else {
				return static_cast<const UnrefU&&>(Ux);
			}
		}
		else {
			if constexpr (std::is_lvalue_reference_v<Ty>) {
				return static_cast<UnrefU&>(Ux);
			}
			else {
				return static_cast<UnrefU&&>(Ux);
			}
		}
	}
#endif

#if __cpp_lib_byteswap < 202110L

	[[nodiscard]] constexpr unsigned short byteswap_ushort(const unsigned short val) noexcept {
		return static_cast<unsigned short>((val << 8) | (val >> 8));
	}

	[[nodiscard]] constexpr unsigned long byteswap_ulong(const unsigned long val) noexcept {
		return (val << 24) | ((val << 8) & 0x00FF'0000) | ((val >> 8) & 0x0000'FF00) | (val >> 24);
	}

	[[nodiscard]] constexpr unsigned long long byteswap_uint64(const unsigned long long val) noexcept {
		return (val << 56) | ((val << 40) & 0x00FF'0000'0000'0000) | ((val << 24) & 0x0000'FF00'0000'0000)
			| ((val << 8) & 0x0000'00FF'0000'0000) | ((val >> 8) & 0x0000'0000'FF00'0000)
			| ((val >> 24) & 0x0000'0000'00FF'0000) | ((val >> 40) & 0x0000'0000'0000'FF00) | (val >> 56);
	}

	template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	[[nodiscard]] constexpr T byteswap(const T val) noexcept
	{
		if constexpr (sizeof(T) == 1)
			return val;
		else if constexpr (sizeof(T) == 2)
			return static_cast<T>(byteswap_ushort(static_cast<unsigned short>(val)));
		else if constexpr (sizeof(T) == 4)
			return static_cast<T>(byteswap_ulong(static_cast<unsigned long>(val)));
		else if constexpr (sizeof(T) == 8)
			return static_cast<T>(byteswap_uint64(static_cast<unsigned long long>(val)));
		else
			static_assert(always_false<T>, "unsupported integer size");
	}
#else
	using std::byteswap;
#endif
	
#if defined(__cpp_lib_unreachable)
	using std::unreachable;
#else
	[[noreturn]] inline void unreachable() noexcept
	{
		__assume(false);
	}
#endif
}
