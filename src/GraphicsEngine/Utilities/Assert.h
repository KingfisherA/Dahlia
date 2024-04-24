#pragma once
#include <iostream>

#ifdef PLATFORM_WIN
#include "GraphicsEngine/Platform/Windows/Console.h"

#define UNI_ASSERT(condition, object, message)                                             \
    if (condition)                                                                         \
    {                                                                                      \
        GEng::Console::SetConsoleColour(GEng::ConsoleColour::Red);                         \
        std::cerr << "Assertion Failed! [" << #object << "] Message: " << message << '\n'; \
    }
#define UNI_FATAL(object, message)                                   \
    GEng::Console::SetConsoleColour(GEng::ConsoleColour::BrightRed); \
    std::cerr << "[" << #object << "] Message: " << message << '\n';
#define UNI_WARN(object, message)                                       \
    GEng::Console::SetConsoleColour(GEng::ConsoleColour::BrightYellow); \
    std::cerr << "[" << #object << "] Message: " << message << '\n';

#define UNI_LOG(col, object, message)                          \
    GEng::Console::SetConsoleColour(GEng::ConsoleColour::col); \
    std::cerr << '[' << #object << ']' << ' ' << message << '\n';

#else

#define UNI_ASSERT(condition, object, message)                                             \
    if (condition)                                                                         \
    {                                                                                      \
        std::cerr << "Assertion Failed! [" << #object << "] Message: " << message << '\n'; \
    }
#define UNI_FATAL(object, message) std::cerr << "[" << #object << "] Message: " << message << '\n';
#define UNI_WARN(object, message) std::cerr << "[" << #object << "] Message: " << message << '\n';

#define UNI_LOG(col, object, message) std::cerr << '[' << #object << ']' << ' ' << message << '\n';

#endif
