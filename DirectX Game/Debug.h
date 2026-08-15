#pragma once
#include <Windows.h>
#include <cstdio>
#include <cstdarg>

// Simple logging macros. LOG_INFO/LOG_ERROR always emit. LOG_DEBUG compiles out in NDEBUG builds.

#ifdef NDEBUG
    // Release build: disable debug logs
    #define LOG_DEBUG(...) ((void)0)
#else
    inline void LOG_DEBUG(const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
        va_end(args);
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");
    }
#endif

inline void LOG_INFO(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
}

inline void LOG_ERROR(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
}

// Backwards compatible alias
#define LOG LOG_INFO