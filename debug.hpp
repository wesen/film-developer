#pragma once
#include <cstdio>

#ifdef NDEBUG
#define DEBUG_PRINT(fmt, ...) ((void)0)
#define DEBUG_PRINTN(fmt, ...) ((void)0)
#else
#define DEBUG_PRINT(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define DEBUG_PRINTN(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif
