#include <chino/utf8.hpp>
#include <chino/test.hpp>

inline auto test_validate (chino::test::test & t)
{
  using namespace std::literals;
  {
    // valid
    auto a = u8"aαあ🍀\uFFFD\0\u007F\u0080\u0081\u07FF\u0800\u0801\uFFFF\U00010000\U00010001\U0010FFFFinudex emiawp/fea3$A&&$5\t\n\v\a\b\f\\"sv;
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, std::u8string_view::npos);
  }
  {
    // 0-indexedで0バイト目が不正
    for (char8_t i = 0b10000000; i != 0b11111111; i |= i >> 1)
    {
      std::u8string a {i};
      auto b = chino::utf8::find_invalid (a);
      t.assert_eq (b, 0zu);
    }
    t.assert_eq (chino::utf8::find_invalid (std::u8string {0xFF}), 0zu);
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xC0, 0xAF}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 0zu);
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xE0, 0x80, 0xAF}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 0zu);
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xF0, 0x80, 0x80, 0xAF}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 0zu);
  }
  {
    // 途中で終わっている
    std::u8string a {{0xC3, 0x2F}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 0zu);
  }
  {
    // サロゲート用コード値
    std::u8string a = {{0xED, 0xA0, 0x80}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 0zu);
  }
  {
    // サロゲート用コード値
    std::u8string a = {{0xED, 0xBF, 0xBF}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 0zu);
  }
  {
    // 途中まで正常
    std::u8string a = {{0xE3, 0x81, 0x82, 0xFF}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 3zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, std::u8string_view::npos);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 8zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 7zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 6zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 5zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 4zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 3zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 2zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::find_invalid (a);
    t.assert_eq (b, 1zu);
  }
}

auto main () -> int
{
  int res = 0;
  res |= chino::test::add_test (test_validate);
  return res;
}
