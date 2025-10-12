#pragma once
#include <iostream>

#ifdef PLATFORM_WIN
#include "Dahlia/Platform/Windows/Console.h"

#define DH_ASSERT(condition, object, message)                                             \
    if (condition)                                                                         \
    {                                                                                      \
        DH::Console::SetConsoleColour(DH::ConsoleColour::Red);                         \
        std::cerr << "Assertion Failed! [" << #object << "]  " << message << '\n'; \
    }
#define DH_FATAL(object, message)                                   \
    DH::Console::SetConsoleColour(DH::ConsoleColour::BrightRed); \
    std::cerr << "[" << #object << "]  " << message << '\n';
#define DH_WARN(object, message)                                       \
    DH::Console::SetConsoleColour(DH::ConsoleColour::BrightYellow); \
    std::cerr << "[" << #object << "]  " << message << '\n';

#define DH_LOG(col, object, message)                          \
    DH::Console::SetConsoleColour(DH::ConsoleColour::col); \
    std::cerr << '[' << #object << ']' << ' ' << message << '\n';

#else

#define DH_ASSERT(condition, object, message)                                             \
    if (condition)                                                                         \
    {                                                                                      \
        std::cerr << "Assertion Failed! [" << #object << "] " << message << '\n'; \
        throw std::runtime_error("");\
    }
#define DH_FATAL(object, message) std::cerr << '[' << #object << "] " << message << '\n';
#define DH_WARN(object, message) std::cerr << '[' << #object << "] " << message << '\n';

#define DH_LOG(col, object, message) std::cerr << '[' << #object << "] " << message << '\n';

#endif
