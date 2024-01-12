#pragma once

#include <concepts>
#include <limits>
#include <cmath>
#include <bit>

namespace ghassanpl::constexpr_math {}
namespace ghassanpl { namespace cem = ghassanpl::constexpr_math; }

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
	using std::isnan;
#else

	constexpr bool isnan(std::integral auto f) noexcept { return false; }
	constexpr bool isnan(float f) { 
		if (std::is_constant_evaluated())
			return (std::bit_cast<uint32_t>(f) & 0x7FFFFFFFu) > 0x7F800000u;
		else
			return std::isnan(f);
	}
	constexpr bool isnan(double f) { 
		if (std::is_constant_evaluated())
			return (std::bit_cast<uint64_t>(f) & 0x7FFFFFFFFFFFFFFFull) > 0x7FF0000000000000ull;
		else
			return std::isnan(f);
	}
	constexpr bool isfinite(float f) {
		if (std::is_constant_evaluated())
			return (std::bit_cast<uint32_t>(f) & 0x7F800000u) != 0x7F800000u;
		else
			return std::isfinite(f);
	}
	constexpr bool isfinite(double f) {
		if (std::is_constant_evaluated())
			return (std::bit_cast<uint64_t>(f) & 0x7FF0000000000000ull) != 0x7FF0000000000000ull;
		else
			return std::isfinite(f);
	}

	/// Shamelessly stolen from https://gist.github.com/Redchards/7f1b357bf3e686b55362

	template <std::floating_point T, typename RESULT = T>
	constexpr RESULT floor(T num) noexcept
	{
		if (std::is_constant_evaluated())
		{
			if (num == -std::numeric_limits<T>::infinity())
				return static_cast<RESULT>(-std::numeric_limits<T>::infinity());
			else if (num == std::numeric_limits<T>::infinity())
				return static_cast<RESULT>(std::numeric_limits<T>::infinity());
			else if (num != num)
				return static_cast<RESULT>(std::numeric_limits<T>::quiet_NaN());

			const auto res = static_cast<int64_t>(num);
			return static_cast<RESULT>((res > num) ? res - 1 : res);
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
		if (::ghassanpl::cem::signbit(val))
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
			if (num == -std::numeric_limits<T>::infinity())
				return static_cast<RESULT>(-std::numeric_limits<T>::infinity());
			else if (num == std::numeric_limits<T>::infinity())
				return static_cast<RESULT>(std::numeric_limits<T>::infinity());
			else if (::ghassanpl::cem::isnan(num))
				return static_cast<RESULT>(std::numeric_limits<T>::quiet_NaN());

			const auto res = static_cast<int64_t>(num);
			return static_cast<RESULT>((res >= num) ? res : res + 1);
		}
		else
			return std::ceil(num);
	}

	template <std::floating_point T, typename RESULT = T>
	constexpr RESULT trunc(T num) noexcept
	{
		if (std::is_constant_evaluated())
			return num < T{} ? -::ghassanpl::cem::floor<T, RESULT>(-num) : ::ghassanpl::cem::floor<T, RESULT>(num);
		else
			return std::trunc(num);
	}

	/// TODO: round

	template <arithmetic T>
	constexpr auto abs(T num) /// can throw
	{
		if (std::is_constant_evaluated())
			return ::ghassanpl::cem::signbit(num) ? -num : num;
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

	template <arithmetic T, arithmetic U>
	constexpr auto fmod(T a, U b) noexcept
	{
		using CT = std::conditional_t<
			std::floating_point<T> || std::floating_point<U>,
			decltype(fma(trunc(a / b), -b, a)),
			double
		>;
		if (std::is_constant_evaluated())
		{
			if (b == 0)
				return std::numeric_limits<CT>::quiet_NaN();
			else if (a == 0)
				return CT{};
			else if (a == std::numeric_limits<T>::infinity() || a == -std::numeric_limits<T>::infinity())
				return std::numeric_limits<CT>::quiet_NaN();
			else if (b == std::numeric_limits<U>::infinity() || b == -std::numeric_limits<U>::infinity())
				return CT{};
			else if (::ghassanpl::cem::isnan(a) || ::ghassanpl::cem::isnan(b))
				return std::numeric_limits<CT>::quiet_NaN();

			return ::ghassanpl::cem::fma(::ghassanpl::cem::trunc((CT)a / (CT)b), -(CT)b, (CT)a);
		}
		else
			return std::fmod((CT)a, (CT)b);
	}
#endif

	#include "constexpr_math.inl"

	template <arithmetic T>
	constexpr auto sqrt(T value) noexcept
	{
		using CT = std::conditional_t<std::floating_point<T>, T, double>;
		if (std::is_constant_evaluated())
		{
			static_assert(std::numeric_limits<double>::is_iec559, "sqrt only works for IEEE 754 floating point types");
			return (CT)::ghassanpl::cem::detail::sqrt_impl((double)value);
		}
		else
			return (CT)std::sqrt(value);
	}

	template <arithmetic T, arithmetic U>
	constexpr auto pow(T base, U exponent) noexcept
	{
		using CT = decltype(base * exponent);
		if (std::is_constant_evaluated())
		{
			if constexpr (std::integral<T> && std::integral<U>)
			{
				CT result = static_cast<CT>(1);
				while (exponent-- > 0)
					result = result * base;
				return result;
			}
			else
			{
				static_assert(std::numeric_limits<double>::is_iec559, "pow only works for IEEE 754 floating point types");
				return (CT)::ghassanpl::cem::detail::pow_impl((double)base, (double)exponent);
			}
		}
		else
			return (CT)std::pow(base, exponent);
	}

	template <std::integral T>
	constexpr unsigned ilog2(T val) noexcept
	{
		unsigned result = 0;
		if (val >= (1ULL << 32)) { result += 32; val >>= 32ULL; }
		if (val >= (1ULL << 16)) { result += 16; val >>= 16ULL; }
		if (val >= (1ULL << 8)) { result += 8; val >>= 8ULL; }
		if (val >= (1ULL << 4)) { result += 4; val >>= 4ULL; }
		if (val >= (1ULL << 2)) { result += 2; val >>= 2ULL; }
		if (val >= (1ULL << 1)) result += 1;
		return result;
	}
}
