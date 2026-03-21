/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>	
#include <string>

struct UnCopyable
{
  constexpr UnCopyable() noexcept = default;
  constexpr UnCopyable(UnCopyable const&) noexcept = delete;
  constexpr UnCopyable(UnCopyable&&) noexcept = default;
  constexpr UnCopyable& operator=(UnCopyable const&) noexcept = delete;
  constexpr UnCopyable& operator=(UnCopyable&&) noexcept = default;

  constexpr bool operator==(UnCopyable const&) const noexcept { return true; }
};

inline constexpr UnCopyable uncopyable{};

struct UnMovable
{
  constexpr UnMovable() noexcept = default;
  constexpr UnMovable(UnMovable const&) noexcept = delete;
  constexpr UnMovable(UnMovable&&) noexcept = delete;
  constexpr UnMovable& operator=(UnMovable const&) noexcept = delete;
  constexpr UnMovable& operator=(UnMovable&&) noexcept = delete;

  constexpr bool operator==(UnMovable const&) const noexcept { return true; }
};

inline constexpr UnMovable unmovable{};

inline std::string to_string(UnCopyable const& cp) { return "UnCopyable"; }
inline std::string to_string(UnMovable const& cp) { return "UnMovable"; }

using std::ignore;

using integer_types = ::testing::Types <
	short int, unsigned short int, int, unsigned int, long int, unsigned long int, long long int, unsigned long long int,
	signed char, unsigned char, char, wchar_t, char16_t, char32_t
#if __cplusplus > 201703L
	, char8_t
#endif
> ;

template <typename RESULT_TYPE>
class bits_test : public ::testing::Test {
public:

	using result_type = RESULT_TYPE;

};