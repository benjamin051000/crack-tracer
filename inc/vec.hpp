#pragma once
#include <cstdint>
#include <immintrin.h>

template <typename DataType>
struct _Vec3 {
  DataType x, y, z;
};

// TODO can I use a default template typename (= float) in the declaration? I keep seeing weird issues in usage (Having to specify `Vec<>`, etc.) Ugh
using Vec3 = _Vec3<float>;

using Vec3_256 = _Vec3<__m256>;
using CharColor = _Vec3<uint8_t>;
