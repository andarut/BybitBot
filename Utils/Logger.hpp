#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <utility>
#include <cstdarg>

// #define TRUEDEBUG

enum class LogLevel : int { DEBUG=0, INFO, WARN, ERROR };

static constexpr const char* level_names[] = {
    "DEBUG", "INFO", "WARN", "ERROR"
};

static constexpr const char* RESET  = "\033[0m";

/* Colors */
static constexpr const char* RED    = "\033[31m";
static constexpr const char* GREEN  = "\033[32m";
static constexpr const char* YELLOW = "\033[33m";
static constexpr const char* BLUE   = "\033[36m";

/* Text format */
static constexpr const char* HEADER    = "\033[35m";
static constexpr const char* UNDERLINE = "\033[4m";
static constexpr const char* BOLD      = "\033[1m";

template<LogLevel L>
inline void log(const char* file, const char* func, int line, const char* fmt, ...) {
    const char* color = L==LogLevel::ERROR ? RED :
                        L==LogLevel::WARN  ? YELLOW :
                        L==LogLevel::INFO  ? GREEN : RESET;
    std::va_list args;
    va_start(args, fmt);
    std::fprintf(stderr, "%s[%s]%s[%s:%d] ",
        color,
        level_names[static_cast<int>(L)],
        RESET,
        func, line);
    std::vfprintf(stderr, fmt, args);
    std::fprintf(stderr, "%s", RESET);
    va_end(args);

}

#ifndef NDEBUG

#ifdef TRUEDEBUG
#define DEBUG(fmt, ...) log<LogLevel::DEBUG>(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif // TRUEDEBUG

#define  INFO(fmt, ...) log<LogLevel::INFO >(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define  WARN(fmt, ...) log<LogLevel::WARN >(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) log<LogLevel::ERROR>(__FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)

#else

#define DEBUG(fmt, ...)
#define  INFO(fmt, ...)
#define  WARN(fmt, ...)
#define ERROR(fmt, ...)

#endif  // NDEBUG

#endif  // LOGGER_H
