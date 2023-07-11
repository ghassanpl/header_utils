#pragma once

#include <cstdint>

/// Because GCC and Clang suck

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
#include <source_location>
#else
namespace std
{
	struct source_location
	{
        [[nodiscard]] static consteval source_location current(const uint_least32_t _Line_ = __builtin_LINE(),
            const uint_least32_t _Column_ = __builtin_COLUMN(), const char* const _File_ = __builtin_FILE(),
#if defined(__clang__) || defined(__EDG__) // TRANSITION, DevCom-10199227 and LLVM-58951
            const char* const _Function_ = __builtin_FUNCTION()
#else // ^^^ workaround / no workaround vvv
            const char* const _Function_ = __builtin_FUNCSIG()
#endif // TRANSITION, DevCom-10199227 and LLVM-58951
        ) noexcept {
            source_location _Result{};
            _Result._Line = _Line_;
            _Result._Column = _Column_;
            _Result._File = _File_;
            _Result._Function = _Function_;
            return _Result;
        }

        [[nodiscard]] constexpr source_location() noexcept = default;

        [[nodiscard]] constexpr uint_least32_t line() const noexcept {
            return _Line;
        }
        [[nodiscard]] constexpr uint_least32_t column() const noexcept {
            return _Column;
        }
        [[nodiscard]] constexpr const char* file_name() const noexcept {
            return _File;
        }
        [[nodiscard]] constexpr const char* function_name() const noexcept {
            return _Function;
        }

    private:
        uint_least32_t _Line{};
        uint_least32_t _Column{};
        const char* _File = "";
        const char* _Function = "";
    };
}
#endif