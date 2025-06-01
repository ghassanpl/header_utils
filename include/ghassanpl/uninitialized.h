#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

/// Based on https://github.com/oracle-9/maybe_uninit/ so I guess this file is specifically under
/// the GPL 3 license: https://github.com/oracle-9/maybe_uninit/blob/main/LICENSE

namespace ghassanpl
{
	namespace detail
	{
		template <typename T>
		concept default_constructible = requires { ::new (static_cast<void*>(std::declval<T*>())) T; };

		template <typename T, typename... Args>
		concept paren_constructible_from = requires(Args&&... args) {
			::new (static_cast<void*>(std::declval<T*>())) T(std::forward<Args>(args)...);
		};

		template <typename T, typename... Args>
		concept brace_constructible_from = requires(Args&&... args) {
			::new (static_cast<void*>(std::declval<T*>())) T{ std::forward<Args>(args)... };
		};

		template <typename T>
		concept nothrow_default_constructible = noexcept(::new (static_cast<void*>(std::declval<T*>())) T);

		template <typename T, typename... Args>
		concept nothrow_paren_constructible_from = requires(Args&&... args) {
			{ ::new (static_cast<void*>(std::declval<T*>())) T(std::forward<Args>(args)...) } noexcept;
		};

		template <typename T, typename... Args>
		concept nothrow_brace_constructible_from = requires(Args&&... args) {
			{ ::new (static_cast<void*>(std::declval<T*>())) T{ std::forward<Args>(args)... } } noexcept;
		};

		template <typename T>
		concept sized = std::is_object_v<T> and requires { sizeof(T); };

		template <typename T>
		concept const_ref = std::is_reference_v<T> and std::is_const_v<std::remove_reference_t<T>>;
	}

	struct default_init_t {} default_init;
	struct paren_init_t {} paren_init;
	struct brace_init_t {} brace_init;

	template <detail::sized T>
	union unititialized_t
	{
	public:
		constexpr unititialized_t() noexcept {}

		explicit constexpr unititialized_t(default_init_t)
			noexcept(detail::nothrow_default_constructible<T>)
			requires detail::default_constructible<T>
		{
			this->default_init();
		}

		template <typename... Args>
		explicit constexpr unititialized_t(paren_init_t, Args&&... args)
			noexcept(detail::nothrow_paren_constructible_from<T, Args...>)
			requires detail::paren_constructible_from<T, Args...>
		{
			this->paren_init(std::forward<Args>(args)...);
		}

		template <typename... Args>
		explicit constexpr unititialized_t(brace_init_t, Args&&... args)
			noexcept(detail::nothrow_brace_constructible_from<T, Args...>)
			requires detail::brace_constructible_from<T, Args...>
		{
			this->brace_init(std::forward<Args>(args)...);
		}

		constexpr ~unititialized_t()
			requires std::is_trivially_destructible_v<T>
		= default;

		constexpr ~unititialized_t() {}

		template <typename Self>
		constexpr auto default_init(this Self&& self)
			noexcept(detail::nothrow_default_constructible<T>) -> T&
			requires detail::default_constructible<T>
		{
			return *::new (static_cast<void*>(std::addressof(self.object))) T;
		}

		template <typename Self, typename... Args>
		constexpr auto paren_init(this Self&& self, Args&&... args)
			noexcept(detail::nothrow_paren_constructible_from<T, Args...>) -> T&
			requires detail::paren_constructible_from<T, Args...>
		{
			return *::new (static_cast<void*>(std::addressof(self.object)))
				T(std::forward<Args>(args)...);
		}

		template <typename Self, typename... Args>
		constexpr auto brace_init(this Self&& self, Args&&... args)
			noexcept(detail::nothrow_brace_constructible_from<T, Args...>) -> T&
			requires detail::brace_constructible_from<T, Args...>
		{
			return *::new (static_cast<void*>(std::addressof(self.object)))
				T{ std::forward<Args>(args)... };
		}

		template <typename Self>
		[[nodiscard]] constexpr auto ptr(this Self&& self) noexcept -> auto*
		{
			return std::addressof(self.object);
		}

		template <typename Self>
		[[nodiscard]] constexpr auto operator->(this Self&& self) noexcept -> auto*
		{
			return std::addressof(self.object);
		}

		template <typename Self>
		[[nodiscard]] constexpr auto ref(this Self&& self) noexcept -> auto&&
		{
			return std::forward<Self>(self).object;
		}

		template <typename Self>
		[[nodiscard]] constexpr auto operator*(this Self&& self) noexcept -> auto&&
		{
			return std::forward<Self>(self).object;
		}

		template <typename Self>
		[[nodiscard]] constexpr auto bytes(this Self&& self) noexcept -> std::ranges::borrowed_range auto
		{
			using byte_type = std::conditional_t<
				detail::const_ref<Self> or std::is_const_v<T>,
				std::byte const,
				std::byte>;
			return std::span<byte_type, sizeof(T)>(
				reinterpret_cast<byte_type*>(std::addressof(self.object)),
				sizeof(T)
			);
		}

		template <typename Self>
		constexpr auto destroy(this Self&& self) noexcept(std::is_nothrow_destructible_v<T>)
		{
			if constexpr (not std::is_trivially_destructible_v<T>)
				std::destroy_at(std::addressof(self.object));
		}

	private:
		T object;
	};

	template <detail::sized T>
	unititialized_t(paren_init_t, T) -> unititialized_t<T>;

	template <detail::sized T>
	unititialized_t(brace_init_t, T) -> unititialized_t<T>;
}
