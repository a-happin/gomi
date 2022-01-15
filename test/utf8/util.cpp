#include <chino/utf8.hpp>
#include <chino/test.hpp>

inline auto test_utf8_char_width (chino::test::test & t)
{
  using chino::utf8::char_width;
  {
    char8_t a = 0;
    auto b = char_width (a);
    t.assert_eq (b, 1zu);
  }
  {
    char8_t a = u8'a';
    auto b = char_width (a);
    t.assert_eq (b, 1zu);
  }
  {
    char8_t a = u8"α"[0];
    auto b = char_width (a);
    t.assert_eq (b, 2zu);
  }
  {
    char8_t a = u8"あ"[0];
    auto b = char_width (a);
    t.assert_eq (b, 3zu);
  }
  {
    char8_t a = u8"🍀"[0];
    auto b = char_width (a);
    t.assert_eq (b, 4zu);
  }
  {
    char8_t a = 0xFF;
    auto b = char_width (a);
    t.assert_eq (b, 0zu);
  }
  {
    char8_t a = 0xFE;
    auto b = char_width (a);
    t.assert_eq (b, 0zu);
  }
  {
    char8_t a = 0b10000000;
    auto b = char_width (a);
    t.assert_eq (b, 0zu);
  }
}

inline auto test_is_subsequent (chino::test::test & t)
{
  using chino::utf8::is_subsequent;
  using namespace std::literals;
  {
    char8_t a = 0;
    auto b = is_subsequent (a);
    t.assert_eq (b, false);
  }
  {
    char8_t a = u8'a';
    auto b = is_subsequent (a);
    t.assert_eq (b, false);
  }
  {
    auto a = u8"α"sv;
    for (auto i = 0; auto && c : a)
    {
      auto b = is_subsequent (c);
      t.assert_eq (b, i != 0);
      ++ i;
    }
  }
  {
    auto a = u8"あ"sv;
    for (auto i = 0; auto && c : a)
    {
      auto b = is_subsequent (c);
      t.assert_eq (b, i != 0);
      ++ i;
    }
  }
  {
    auto a = u8"🍀"sv;
    for (auto i = 0; auto && c : a)
    {
      auto b = is_subsequent (c);
      t.assert_eq (b, i != 0);
      ++ i;
    }
  }
  {
    char8_t a = 0xFF;
    auto b = is_subsequent (a);
    t.assert_eq (b, false);
  }
  {
    char8_t a = 0xFE;
    auto b = is_subsequent (a);
    t.assert_eq (b, false);
  }
  {
    char8_t a = 0b10000000;
    auto b = is_subsequent (a);
    t.assert_eq (b, true);
  }
}

auto main () -> int
{
  int res = 0;
  res |= chino::test::add_test (test_utf8_char_width);
  res |= chino::test::add_test (test_is_subsequent);
  return res;
}
