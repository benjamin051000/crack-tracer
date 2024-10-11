#pragma once
#include <immintrin.h>

[[gnu::always_inline]] inline void print_vec_256(const __m256& vec) noexcept;
[[gnu::always_inline]] inline void print_vec_128(const __m128& vec) noexcept;
