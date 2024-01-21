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
