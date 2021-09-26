#include <bool.hpp>

auto main () -> int
{
  using namespace gomi;
  static_assert (nand_ <false , false> {} , "");
  static_assert (nand_ <true , false> {} , "");
  static_assert (nand_ <false , true> {} , "");
  static_assert (! nand_ <true , true> {} , "");

  static_assert (! and_ <false , false> {} , "");
  static_assert (! and_ <true , false> {} , "");
  static_assert (! and_ <false , true> {} , "");
  static_assert (and_ <true , true> {} , "");

  static_assert (! or_ <false , false> {} , "");
  static_assert (or_ <true , false> {} , "");
  static_assert (or_ <false , true> {} , "");
  static_assert (or_ <true , true> {} , "");

  static_assert (nor_ <false , false> {} , "");
  static_assert (! nor_ <true , false> {} , "");
  static_assert (! nor_ <false , true> {} , "");
  static_assert (! nor_ <true , true> {} , "");
}
