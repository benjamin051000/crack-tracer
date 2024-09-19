#include "utils.hpp"

void print_vec_256(const __m256 vec) {

  printf("%.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n", vec[0], vec[1], vec[2], vec[3], vec[4],
         vec[5], vec[6], vec[7]);
}

void print_vec_128(const __m128 vec) {
  printf("%.3f %.3f %.3f %.3f\n", vec[0], vec[1], vec[2], vec[3]);
}
