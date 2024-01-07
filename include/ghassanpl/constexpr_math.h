#pragma once

#include <concepts>
#include <limits>
#include <cmath>
#include <bit>

namespace ghassanpl::constexpr_math
{
	template <typename T>
	concept arithmetic = std::is_arithmetic_v<T>;

#ifdef __cpp_lib_constexpr_cmath
	using std::pow;
	using std::floor;
	using std::sign;
	using std::signbit;
	using std::ceil;
	using std::trunc;
	using std::abs;
	using std::fma;
	using std::fmod;
#else

	/// Shamelessly stolen from https://gist.github.com/Redchards/7f1b357bf3e686b55362

	/// TODO: negative exponents
	template <typename T, typename E>
	constexpr auto pow(T base, E exponent) noexcept
	{
		static_assert(std::integral<E> && std::is_unsigned_v<E>, "exponent must be of unsigned integral type");

		if (std::is_constant_evaluated())
		{
			T result = static_cast<T>(1);
			while (exponent-- > 0)
				result = result * base;
			return result;
		}
		else
			return (T)std::pow(base, (T)exponent);
	}

	template <size_t N, typename T>
	constexpr auto pow(T base) noexcept
	{
		if (std::is_constant_evaluated())
		{
			T result = static_cast<T>(1);
			for (size_t i = 0; i < N; ++i)
				result = result * base;
			return result;
		}
		else
			return std::pow(base, (T)N);
	}

	template <std::floating_point T, typename RESULT = T>
	constexpr RESULT floor(T num) noexcept
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
	constexpr bool signbit(T num) /// can throw
	{
		if (std::is_constant_evaluated())
		{
			if constexpr (std::floating_point<T> && std::numeric_limits<T>::is_iec559)
			{
				/// Works for inf and nan
				if constexpr (sizeof(T) == sizeof(uint64_t))
					return (std::bit_cast<uint64_t>(num) & 0x8000000000000000ull) != 0;
				else if constexpr (sizeof(T) == sizeof(uint32_t))
					return (std::bit_cast<uint32_t>(num) & 0x80000000u) != 0;
				else if constexpr (sizeof(T) == sizeof(uint16_t))
					return (std::bit_cast<uint16_t>(num) & 0x8000u) != 0;
				else if constexpr (sizeof(T) == sizeof(uint8_t))
					return (std::bit_cast<uint8_t>(num) & 0x80u) != 0;
				else
					static_assert(sizeof(T) == sizeof(uint8_t), "unsupported floating point type");
			}
			else
			{
				return num < T{};
			}
		}
		else
			return std::signbit(num);
	}

	template <typename T>
	requires (!std::is_signed_v<T>) /// Accepts both unsigned and non-builtin types
	constexpr bool signbit(T val) noexcept
	{
		return val < T{};
	}

	template <typename T>
	constexpr int sign(T val)
	{
		if (signbit(val))
			return -1;
		else if (val == T{})
			return 0;
		return 1;
	}

	/// TODO: sign
	/// return num > 0 ? 1 : (num < 0 ? -1 : 0);

	template <std::floating_point T, typename RESULT = T>
	constexpr RESULT ceil(T num) noexcept
	{
		if (std::is_constant_evaluated())
		{
			const auto res = static_cast<int64_t>(num);
			return static_cast<RESULT>((res > num) ? res : res + 1);
		}
		else
			return std::ceil(num);
	}

	template <std::floating_point T, typename RESULT = T>
	constexpr RESULT trunc(T num) noexcept
	{
		if (std::is_constant_evaluated())
		{
			return num < T{} ? -floor<T, RESULT>(-num) : floor<T, RESULT>(num);
		}
		else
			return std::trunc(num);
	}

	/// TODO: round

	template <arithmetic T>
	constexpr auto abs(T num) /// can throw
	{
		if (std::is_constant_evaluated())
			return signbit(num) ? -num : num;
		else if constexpr (std::is_signed_v<T>)
			return std::abs(num);
		else
			return num;
	}

	template <arithmetic T, arithmetic U, arithmetic V>
	constexpr auto fma(T a, U b, V c) noexcept
	{
		if (std::is_constant_evaluated())
			return a * b + c;
		else
		{
			using CT = std::conditional_t<
				std::floating_point<T> || std::floating_point<U> || std::floating_point<V>,
				decltype(a* b + c),
				double
			>;
			return std::fma((CT)a, (CT)b, (CT)c);
		}
	}

	/// TODO: 
	/// template <typename T, typename U>
	/// requires std::floating_point<T> || std::floating_point<U>
	/// auto fmod(T a, U b) -> decltype(<calc>) { 
	///		using CT = decltype(<calc>);
	///		return ((CT)<calc>);
	/// }
	template <arithmetic T, arithmetic U>
	constexpr auto fmod(T a, U b) noexcept
	{
		using CT = std::conditional_t<
			std::floating_point<T> || std::floating_point<U>,
			decltype(fma(trunc(a / b), -b, a)),
			double
		>;
		if (std::is_constant_evaluated())
			return fma(trunc((CT)a / (CT)b), -(CT)b, (CT)a);
		else
			return std::fmod((CT)a, (CT)b);
	}
#endif
}

namespace ghassanpl
{
	namespace cem = ghassanpl::constexpr_math;
}