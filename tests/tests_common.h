#pragma once

#include <string>

struct UnCopyable
{
  UnCopyable() = default;
  UnCopyable(UnCopyable const&) = delete;
  UnCopyable(UnCopyable&&) = default;
  UnCopyable& operator=(UnCopyable const&) = delete;
  UnCopyable& operator=(UnCopyable&&) = default;

  bool operator==(UnCopyable const& other) const noexcept { return true; }
};

struct UnMovable
{
  UnMovable() = default;
  UnMovable(UnMovable const&) = delete;
  UnMovable(UnMovable&&) = delete;
  UnMovable& operator=(UnMovable const&) = delete;
  UnMovable& operator=(UnMovable&&) = delete;

  bool operator==(UnMovable const& other) const noexcept { return true; }
};

inline std::string to_string(UnCopyable const& cp) { return "UnCopyable"; }
inline std::string to_string(UnMovable const& cp) { return "UnMovable"; }