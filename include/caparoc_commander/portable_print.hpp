#pragma once

// Portable std::println replacement.
// MinGW-w64 GCC 15 does not yet implement the terminal I/O functions
// (std::__open_terminal, std::__write_to_terminal) required by std::print/std::println.
// On Windows we fall back to std::format + iostream; on other platforms we use std::println.

#ifdef _WIN32
#include <format>
#include <iostream>
#include <string_view>

namespace portable {

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args)
{
    std::cout << std::format(fmt, std::forward<Args>(args)...) << '\n';
}

inline void println(std::string_view sv)
{
    std::cout << sv << '\n';
}

template <typename... Args>
void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args)
{
    auto msg = std::format(fmt, std::forward<Args>(args)...);
    if (stream == stderr)
        std::cerr << msg << '\n';
    else
        std::cout << msg << '\n';
}

} // namespace portable

#else
#include <print>

namespace portable {

template <typename... Args>
void println(std::format_string<Args...> fmt, Args&&... args)
{
    std::println(fmt, std::forward<Args>(args)...);
}

inline void println(std::string_view sv)
{
    std::println("{}", sv);
}

template <typename... Args>
void println(std::FILE* stream, std::format_string<Args...> fmt, Args&&... args)
{
    std::println(stream, fmt, std::forward<Args>(args)...);
}

} // namespace portable

#endif
