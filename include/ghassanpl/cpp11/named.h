#pragma once

#include <type_traits>
#include <utility>

namespace ghassanpl
{
	template <typename T, typename TYPE_TAG>
	struct named
	{
		using base_type = T;
		using self_type = named<T, TYPE_TAG>;

		T value{};

		template <typename... ARGS, typename = typename std::enable_if<std::is_constructible<T, ARGS...>::value>::type>
		constexpr explicit named(ARGS&&... args) noexcept(std::is_nothrow_constructible<T, ARGS...>::value) : value(std::forward<ARGS>(args)...) {}

		constexpr named() noexcept(std::is_nothrow_default_constructible<T>::value) = default;
		constexpr named(named const&) noexcept(std::is_nothrow_copy_constructible<T>::value) = default;
		constexpr named(named&&) noexcept(std::is_nothrow_move_constructible<T>::value) = default;
		named& operator=(named const&) noexcept(std::is_nothrow_copy_assignable<T>::value) = default;
		named& operator=(named&&) noexcept(std::is_nothrow_move_assignable<T>::value) = default;

		T* operator->() noexcept { return &value; }
		T const* operator->() const noexcept { return &value; }

		T const& get() const& { return value; }
		T get()&& { return std::move(value); }

		template <typename U>
		constexpr U as() const noexcept { return static_cast<U>(value); }

		constexpr T drop() const noexcept(std::is_nothrow_move_constructible<T>::value) { return std::move(value); }

		constexpr explicit operator bool() const noexcept { return value; }
		constexpr explicit operator base_type() const noexcept { return value; }

		constexpr bool operator <(named const& other) const { return value < other.value; }
		constexpr bool operator <=(named const& other) const { return value <= other.value; }
		constexpr bool operator >(named const& other) const { return value > other.value; }
		constexpr bool operator >=(named const& other) const { return value >= other.value; }
		constexpr bool operator ==(named const& other) const { return value == other.value; }
		constexpr bool operator !=(named const& other) const { return value != other.value; }

		friend constexpr bool operator <(T const& value, named const& other) { return value < other.value; }
		friend constexpr bool operator <=(T const& value, named const& other) { return value <= other.value; }
		friend constexpr bool operator >(T const& value, named const& other) { return value > other.value; }
		friend constexpr bool operator >=(T const& value, named const& other) { return value >= other.value; }
		friend constexpr bool operator ==(T const& value, named const& other) { return value == other.value; }
		friend constexpr bool operator !=(T const& value, named const& other) { return value != other.value; }
	};

}