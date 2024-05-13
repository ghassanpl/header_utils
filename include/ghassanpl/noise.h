/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <cstdint>

/// Shamelessly stolen from https://github.com/SRombauts/SimplexNoise/

namespace ghassanpl::noise
{

	/// Gradient noise

	namespace detail
	{
		template <typename F>
		constexpr int32_t fastfloor(F fp) noexcept
		{
			const auto i = static_cast<int32_t>(fp);
			return (fp < i) ? (i - 1) : i;
		}

		constexpr inline uint8_t perm[256] = {
			151, 160, 137, 91, 90, 15,
			131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
			190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
			88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
			77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
			102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
			135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
			5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
			223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
			129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
			251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
			49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
			138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
		};

		constexpr uint8_t hash(int32_t i) noexcept
		{
			return perm[static_cast<uint8_t>(i)];
		}

		template <typename F>
		constexpr F grad(int32_t hash, F x) noexcept
		{
			const int32_t h = hash & 0x0F;  // Convert low 4 bits of hash code
			F grad = F(1.0) + (h & 7);    // Gradient value 1.0, 2.0, ..., 8.0
			if ((h & 8) != 0) grad = -grad; // Set a random sign for the gradient
			return (grad * x);              // Multiply the gradient with the distance
		}

		template <typename F>
		constexpr F grad(int32_t hash, F x, F y) noexcept
		{
			const int32_t h = hash & 0x3F;  // Convert low 3 bits of hash code
			const F u = h < 4 ? x : y;  // into 8 simple gradient directions,
			const F v = h < 4 ? y : x;
			return ((h & 1) ? -u : u) + ((h & 2) ? F(-2.0) * v : F(2.0) * v); // and compute the dot product with (x,y).
		}

		template <typename F>
		constexpr F grad(int32_t hash, F x, F y, F z) noexcept
		{
			const int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
			const F u = h < 8 ? x : y; // gradient directions, and compute dot product.
			const F v = h < 4 ? y : h == 12 || h == 14 ? x : z; // Fix repeats at h = 12 to 15
			return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
		}
	}

	template <typename F>
	[[nodiscard]] constexpr F simplex_noise(F x) noexcept
	{
		static_assert(std::is_floating_point_v<F>, "simplex_noise only works with floating point arguments");

		int32_t i0 = detail::fastfloor(x);
		int32_t i1 = i0 + 1;

		F x0 = x - i0;
		F x1 = x0 - F(1.0);

		F t0 = F(1.0) - x0 * x0;

		t0 *= t0;
		F n0 = t0 * t0 * detail::grad(detail::hash(i0), x0);

		F t1 = F(1.0) - x1 * x1;

		t1 *= t1;
		F n1 = t1 * t1 * detail::grad(detail::hash(i1), x1);

		return F(0.395) * (n0 + n1);
	}

	template <typename F>
	[[nodiscard]] constexpr F simplex_noise(F x, F y) noexcept
	{
		static_assert(std::is_floating_point_v<F>, "simplex_noise only works with floating point arguments");

		constexpr F F2 = F(0.366025403);  // F2 = (sqrt(3) - 1) / 2
		constexpr F G2 = F(0.211324865);  // G2 = (3 - sqrt(3)) / 6   = F2 / (1 + 2 * K)

		const F s = (x + y) * F2;
		const F xs = x + s;
		const F ys = y + s;
		const int32_t i = fastfloor(xs);
		const int32_t j = fastfloor(ys);

		const F t = static_cast<F>(i + j) * G2;
		const F X0 = i - t;
		const F Y0 = j - t;
		const F x0 = x - X0;
		const F y0 = y - Y0;

		int32_t i1 = 0;
		int32_t j1 =0;
		if (x0 > y0) {
			i1 = 1;
			j1 = 0;
		}
		else {
			i1 = 0;
			j1 = 1;
		}

		const F x1 = x0 - i1 + G2;
		const F y1 = y0 - j1 + G2;
		const F x2 = x0 - F(1.0) + F(2.0) * G2;
		const F y2 = y0 - F(1.0) + F(2.0) * G2;

		const int gi0 = hash(i + hash(j));
		const int gi1 = hash(i + i1 + hash(j + j1));
		const int gi2 = hash(i + 1 + hash(j + 1));

		F n0;
		if (F t0 = F(0.5) - x0 * x0 - y0 * y0; t0 < F(0.0))
			n0 = F(0.0);
		else {
			t0 *= t0;
			n0 = t0 * t0 * grad(gi0, x0, y0);
		}

		F n1;
		if (F t1 = F(0.5) - x1 * x1 - y1 * y1; t1 < F(0.0))
			n1 = F(0.0);
		else {
			t1 *= t1;
			n1 = t1 * t1 * grad(gi1, x1, y1);
		}

		F n2;
		if (F t2 = F(0.5) - x2 * x2 - y2 * y2; t2 < F(0.0))
			n2 = F(0.0);
		else {
			t2 *= t2;
			n2 = t2 * t2 * grad(gi2, x2, y2);
		}

		return F(45.23065) * (n0 + n1 + n2);
	}

	template <typename F>
	[[nodiscard]] constexpr F fractal_simplex_noise(size_t octaves, F x, F frequency = F(1.0), F amplitude = F(1.0), F lacunarity = F(2.0), F persistence = F(0.5)) noexcept
	{
		static_assert(std::is_floating_point_v<F>, "fractal_simplex_noise only works with floating point arguments");

		F output = 0;
		F denom = 0;

		for (size_t i = 0; i < octaves; i++)
		{
			output += (amplitude * simplex_noise(x * frequency));
			denom += amplitude;

			frequency *= lacunarity;
			amplitude *= persistence;
		}

		return (output / denom);
	}

	template <typename F>
	[[nodiscard]] constexpr F fractal_simplex_noise_2d(size_t octaves, F x, F y, F frequency = F(1.0), F amplitude = F(1.0), F lacunarity = F(2.0), F persistence = F(0.5)) noexcept
	{
		static_assert(std::is_floating_point_v<F>, "fractal_simplex_noise only works with floating point arguments");

		F output = 0;
		F denom = 0;

		for (size_t i = 0; i < octaves; i++)
		{
			output += (amplitude * simplex_noise(x * frequency, y * frequency));
			denom += amplitude;

			frequency *= lacunarity;
			amplitude *= persistence;
		}

		return (output / denom);
	}

}