#pragma once

#include <concepts>

namespace ghassanpl::constexpr_math
{
#ifdef __cpp_lib_constexpr_cmath
	using std::pow;
	using std::floor;
	using std::sign;
	using std::ceil;
	using std::abs;
	using std::signbit;
	using std::fmod;
#else

	/// Shamelessly stolen from https://gist.github.com/Redchards/7f1b357bf3e686b55362

	template <typename T, std::integral E>
	requires std::is_unsigned_v<E>
	constexpr T pow(T base, E exponent)
	{
		if (std::is_constant_evaluated())
		{
			T result = 1;
			while (exponent-- > 0)
				result = result * base;
			return result;
		}
		else
			return std::pow(base, exponent);
	}

	template <std::floating_point T, typename RESULT = T>
	constexpr RESULT floor(T num)
	{
		if (std::is_constant_evaluated())
		{
			const auto res = static_cast<int64_t>(num);
			return static_cast<RESULT>((res > num) ?  res-1 : res);
		}
		else
			return std::floor(num);
	}

	template <typename T>
	requires std::is_signed_v<T>
	constexpr bool signbit(T num)
	{
		if (std::is_constant_evaluated())
			return num > 0 ? 1 : (num < 0 ? -1 : 0);
		else
			return std::signbit(num);
	}

	template <typename T>
	requires std::is_unsigned_v<T>
	constexpr bool signbit(T num)
	{
		return false;
	}

	template <std::floating_point T, typename RESULT = T>
	constexpr RESULT ceil(T num)
	{
		if (std::is_constant_evaluated())
		{
			const auto res = static_cast<int64_t>(num);
			return static_cast<RESULT>((res > num) ? res : res + 1);
		}
		else
			return std::ceil(num);
	}

	template <typename T>
	requires std::is_arithmetic_v<T> && std::is_signed_v<T>
	constexpr T abs(T num)
	{
		if (std::is_constant_evaluated())
			return signbit(num) ? -num : num;
		else
			return std::abs(num);
	}

	template <std::floating_point T>
	constexpr T fmod(T a, T b)
	{
		if (std::is_constant_evaluated())
			return a - b * floor(a / b);
		else
			return std::fmod(a, b);
	}
#endif
}

namespace ghassanpl
{
	namespace cem = ghassanpl::constexpr_math;
}