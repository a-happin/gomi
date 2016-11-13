#include <integer_sequence.hpp>
#include "debug/show_type.hpp"
#include <iostream>

#define PRINT_TYPE(...) \
  std::cout << debug::show_type <__VA_ARGS__> () << std::endl;

auto main () -> int {
  using namespace gomi;
  PRINT_TYPE (integer_sequence <int , 0>)
  PRINT_TYPE (integer_sequence <size_t , 0>)
  PRINT_TYPE (index_sequence <0 , 1>)

  PRINT_TYPE (range_index_sequence <10 , 13>)
  PRINT_TYPE (make_index_sequence <9>)
  PRINT_TYPE (index_sequence_for <int , int * , int **>)
  PRINT_TYPE (make_index_sequence <300>)
}
