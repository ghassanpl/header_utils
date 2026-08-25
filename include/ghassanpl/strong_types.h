#pragma once

#include <concepts>

/// Shamelessly stolen from https://github.com/JeffGarland/safe_bool/blob/main/include/safe_bool/safe_bool.hpp

namespace ghassanpl
{
	template<typename T>
	concept non_bool_integral = std::integral<T> && !std::same_as<bool, T>;

	class boolean
	{
		bool value = false;
	public:
		constexpr boolean() = default;
		constexpr ~boolean() = default;
		constexpr boolean(const boolean&) = default;
		constexpr boolean(boolean&&) = default;
		constexpr boolean& operator=(boolean&&) = default;

		constexpr explicit(false) boolean(bool v) noexcept : value(v) {}  //not explicit for operator=
		constexpr explicit(false) boolean(non_bool_integral auto) = delete("integral conversion denied");
		constexpr explicit(false) boolean(float) = delete("float conversion denied");
		constexpr explicit(false) boolean(char*) = delete("char ptr conversion denied");

		constexpr boolean& operator=(const boolean& other) = default;

		constexpr explicit operator bool() const noexcept { return value; }

		constexpr bool operator==(const boolean&) const = default;
		template<typename B>
		constexpr bool operator==(B b) const
			requires std::is_same_v<bool, B>
		{
			return b == this->value;
		}

		/// Do these two disable shortcutting?

		constexpr boolean operator&&(const boolean& other) const noexcept {
			return boolean{ this->value && other.value };
		}
		constexpr boolean operator||(const boolean& other) const noexcept {
			return boolean{ this->value || other.value };
		}
	};
}
