#pragma once

#if __has_include(<bit>) && __cpp_lib_bitops
#include <bit>
using std::countl_zero;
using std::countr_zero;
#else
namespace ghassanpl
{
#if defined(__clang__) || defined(__GNUC__)
  inline constexpr int countl_zero(unsigned long long v) { return __builtin_clzll(v); }
  inline constexpr int countr_zero(unsigned long long v) { return __builtin_ctzll(v); }
#elif defined(_MSC_VER)
  extern "C" unsigned char _BitScanReverse(unsigned long* _Index, unsigned long _Mask);
  extern "C" unsigned char _BitScanForward(unsigned long* _Index, unsigned long _Mask);
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)
#if defined(_WIN64)
  extern "C" unsigned char _BitScanReverse64(unsigned long* _Index, unsigned __int64 _Mask);
#pragma intrinsic(_BitScanReverse64)
  extern "C" unsigned char _BitScanForward64(unsigned long* _Index, unsigned __int64 _Mask);
#pragma intrinsic(_BitScanForward64)
#endif
  inline int countl_zero(unsigned long long v)
  {
    unsigned long r = 0;
#if defined(_WIN64)
    _BitScanReverse64(&r, v);
#else
    if (_BitScanReverse(&r, (unsigned long)(v >> 32))) return 31 - r;
    _BitScanReverse(&r, (unsigned long)v);
#endif
    return 63 - r;
  }
  inline int countr_zero(unsigned long long v)
  {
    unsigned long r;
#if defined(_WIN64)
    _BitScanForward64(&r, v);
#else
    if (_BitScanForward64(&r, (unsigned long)(v >> 32))) return r + 32;
    _BitScanForward64(&r, (unsigned long)v);
#endif
    return r;
  }
#else
  inline int constexpr countl_zero(unsigned long long v)
  {
    static constexpr unsigned char MultiplyDeBruijnBitPosition[] = {
       0, 47,  1, 56, 48, 27,  2, 60, 57, 49, 41, 37, 28, 16,  3, 61,
      54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11,  4, 62,
      46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45,
      25, 39, 14, 33, 19, 30,  9, 24, 13, 18,  8, 12,  7,  6,  5, 63
    };
    v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v |= v >> 32;
    return 63 - MultiplyDeBruijnBitPosition[(v * 0x03F79D71B4CB0A89ULL) >> 58];
  }
#endif
}
#endif