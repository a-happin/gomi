#include <chino/char_utils.hpp>

auto main () -> int
{
  using namespace chino::char_utils;

  static_assert (unicode::is_XID_Start (U'a'));
  static_assert (unicode::is_XID_Start (U'α'));
  static_assert (unicode::is_XID_Start (U'あ'));
  static_assert (not unicode::is_XID_Start (U'0'));

  static_assert (unicode::is_XID_Continue (U'a'));
  static_assert (unicode::is_XID_Continue (U'_'));
  static_assert (unicode::is_XID_Continue (U'あ'));
  static_assert (unicode::is_XID_Continue (U'0'));

  static_assert (unicode::is_white_space (U' '));
  static_assert (unicode::is_white_space (U'\t'));
  static_assert (unicode::is_white_space (U'\n'));
  static_assert (unicode::is_white_space (U'\r'));
  static_assert (unicode::is_white_space (U'　'));
}
