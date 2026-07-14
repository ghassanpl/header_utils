/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp20.h"

#if defined(__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L
#include <bit>
#endif
#if defined(__cpp_lib_forward_like)
#include <utility>
#endif
#include <memory>


namespace ghassanpl
{

	template <class T, class = void>
	struct is_complete : std::false_type
	{
	};

	template <class T>
	struct is_complete<T, decltype(void(sizeof(T)))> : std::true_type
	{
	};

	template <class T>
	concept is_complete_v = is_complete<T>::value;

	/*
	template<class T, class D, class U>
	constexpr std::unique_ptr<T, D> dynamic_pointer_cast(std::unique_ptr<U, D> r) noexcept
	{
		if (auto p = dynamic_cast<std::unique_ptr<T, D>::pointer>(r.get()))
			return (void)r.release(), std::unique_ptr<T, D>(p, std::move(r.get_deleter()));

		return std::unique_ptr<T, D>(nullptr, std::move(r.get_deleter()));
	}

	template<class T, class D, class U>
	constexpr std::unique_ptr<T, D> dynamic_steal_cast(std::unique_ptr<U, D>& r) noexcept
	{
		if (auto p = dynamic_cast<std::unique_ptr<T, D>::pointer>(r.get()))
			return (void)r.release(), std::unique_ptr<T, D>(p, std::move(r.get_deleter()));

		return std::unique_ptr<T, D>(nullptr, std::move(r.get_deleter()));
	}

	template<class T, class D, class U>
	constexpr std::unique_ptr<T, D> static_pointer_cast(std::unique_ptr<U, D> r) noexcept
	{
		return std::unique_ptr<T, D>(static_cast<std::unique_ptr<T, D>::pointer>(r.release()), std::move<D>(r.get_deleter()));
	}

	template<class T, class D, class U>
	constexpr std::unique_ptr<T, D> static_steal_cast(std::unique_ptr<U, D>& r) noexcept
	{
		return static_pointer_cast(std::move(r));
	}
	*/

	template<class T, class U>
	constexpr std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<U> r) noexcept
	{
		if (auto p = dynamic_cast<std::unique_ptr<T>::pointer>(r.get()))
			return (void)r.release(), std::unique_ptr<T>(p);

		return std::unique_ptr<T>(nullptr);
	}

	template<class T, class U>
	constexpr std::unique_ptr<T> dynamic_steal_cast(std::unique_ptr<U>& r) noexcept
	{
		return dynamic_pointer_cast<T>(std::exchange(r, {}));
	}

	template<class T, class U>
	constexpr std::unique_ptr<T> static_pointer_cast(std::unique_ptr<U> r) noexcept
	{
		return std::unique_ptr<T>(static_cast<std::unique_ptr<T>::pointer>(r.release()));
	}

	template<class T, class U>
	constexpr std::unique_ptr<T> static_steal_cast(std::unique_ptr<U>& r) noexcept
	{
		return static_pointer_cast<T>(std::move(r));
	}

#ifdef __cpp_concepts
	namespace detail
	{
		template <class T>
		using with_reference = T&;

		template <class T>
		concept can_reference = requires { typename with_reference<T>; };
	}

	template <class T>
	concept dereferenceable = requires(T & t) {
		{ *t } -> detail::can_reference;
	};
#endif

#ifdef __cpp_lib_forward_like
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

#ifdef __cpp_lib_unreachable
	using std::unreachable;
#else
	[[noreturn]] inline void unreachable() noexcept
	{
		__assume(false);
	}
#endif

#ifdef __cpp_lib_start_lifetime_as
	using std::start_lifetime_as;
#else
	template<class T>
	requires (std::is_trivially_copyable_v<T>/* && std::is_implicit_lifetime_v<T> */)
	T* start_lifetime_as(void* p) noexcept
	{
		return std::launder(static_cast<T*>(std::memmove(p, p, sizeof(T))));
	}

	template<class T>
	requires (std::is_trivially_copyable_v<T>/* && std::is_implicit_lifetime_v<T> */)
	T* start_lifetime_as_array(void* p, size_t element_count) noexcept
	{
		return std::launder(static_cast<T*>(std::memmove(p, p, sizeof(T) * element_count)));
	}
#endif

	template<class From, class To>
	concept convertible_without_narrowing = std::is_convertible_v<From, To> &&
		requires (From&& x) {
			{ std::type_identity_t<To[]>{std::forward<From>(x)} } -> std::same_as<To[1]>;
	};

	template <typename T>
	concept has_to_address = requires(const T& val) {
		typename std::pointer_traits<T>;
		std::pointer_traits<T>::to_address(val);
	};

	template <typename T>
	concept pointerlike = std::is_pointer_v<T> or has_to_address<T> or requires(const T & val) {
		val.operator->();
	};

	template <typename T>
	struct pointer_compare_wrapper
	{
		T pointer;

		template <typename U>
		bool operator==(U const& ptr) const noexcept
		{
			using std::to_address;
			return to_address(pointer) == to_address(ptr);
		}
	};

	template <class T> 
	requires std::is_pointer_v<T> 
	pointer_compare_wrapper(T) -> pointer_compare_wrapper<typename std::pointer_traits<T>::element_type*>;

	template <class T> 
	requires (!std::is_pointer_v<T>)
	pointer_compare_wrapper(T) -> pointer_compare_wrapper<T const&>;

	template<typename S, typename D>
	using copy_const_t = std::conditional_t<std::is_const_v<S>, std::add_const_t<D>, std::remove_const_t<D>>;

	template <typename T> T& const_away(const T& v) noexcept { return const_cast<T&>(v); }
	template <typename T> T* const_away(const T* v) noexcept { return const_cast<T*>(v); }

	template <typename T>
	std::shared_ptr<T> make_unmanaged_shared(T* ptr)
	{
		return std::shared_ptr<T>(std::shared_ptr<T>{}, ptr);
	}
}