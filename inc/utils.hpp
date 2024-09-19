#pragma once
#include <cstdio>
#include <immintrin.h>

#ifndef NDEBUG
#define BREAKPOINT asm("int $3");
#else
#define BREAKPOINT ;
#endif

void print_vec_256(const __m256 vec);
void print_vec_128(const __m128 vec);
