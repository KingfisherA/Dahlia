#pragma once

//|| _WIN64 || _W64
#ifdef _WIN32

#ifdef DH_STATIC
#define DH_API
#else
#ifdef DH_BUILD
#define DH_API __declspec(dllexport)
#else
#define DH_API __declspec(dllimport)
#endif
#endif

#else
#define DH_API
#endif