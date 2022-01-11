#include <chino/utf8.hpp>
#include <chino/test.hpp>

inline auto test_utf8_char_width (chino::test::test & t)
{
  using chino::utf8::utf8_char_width;
  {
    char8_t a = 0;
    auto b = utf8_char_width (a);
    t.assert_eq (b, 1zu);
  }
  {
    char8_t a = u8'a';
    auto b = utf8_char_width (a);
    t.assert_eq (b, 1zu);
  }
  {
    char8_t a = u8"α"[0];
    auto b = utf8_char_width (a);
    t.assert_eq (b, 2zu);
  }
  {
    char8_t a = u8"あ"[0];
    auto b = utf8_char_width (a);
    t.assert_eq (b, 3zu);
  }
  {
    char8_t a = u8"🍀"[0];
    auto b = utf8_char_width (a);
    t.assert_eq (b, 4zu);
  }
  {
    char8_t a = 0xFF;
    auto b = utf8_char_width (a);
    t.assert_eq (b, 0zu);
  }
  {
    char8_t a = 0xFE;
    auto b = utf8_char_width (a);
    t.assert_eq (b, 0zu);
  }
  {
    char8_t a = 0b10000000;
    auto b = utf8_char_width (a);
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

inline auto test_unsafe_codepoint (chino::test::test & t)
{
  using chino::utf8::unsafe_codepoint;
  using namespace std::literals;
  {
    auto a = u8"a";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {u8'a'});
  }
  {
    auto a = u8"α";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x3B1});
  }
  {
    auto a = u8"あ";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x3042});
  }
  {
    auto a = u8"🍀";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x1F340});
  }
  {
    auto a = u8"\uFFFD";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0xFFFD});
  }
  {
    std::u8string a {u8'\0'};
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0});
  }
  {
    auto a = u8"\u007F";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x7F});
  }
  {
    auto a = u8"\u0080";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x80});
  }
  {
    auto a = u8"\u0081";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x81});
  }
  {
    auto a = u8"\u07FF";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x7FF});
  }
  {
    auto a = u8"\u0800";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x800});
  }
  {
    auto a = u8"\u0801";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x801});
  }
  {
    auto a = u8"\uFFFF";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0xFFFF});
  }
  {
    auto a = u8"\U00010000";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x10000});
  }
  {
    auto a = u8"\U00010001";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x10001});
  }
  {
    auto a = u8"\U0010FFFF";
    auto b = unsafe_codepoint (a);
    t.assert_eq (b, chino::utf8::codepoint_t {0x10FFFF});
  }
}

inline auto test_codepoint (chino::test::test & t)
{
  using chino::utf8::codepoint;
  using namespace std::literals;
  {
    auto a = u8"a";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {u8'a'}});
  }
  {
    auto a = u8"α";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x3B1}});
  }
  {
    auto a = u8"あ";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x3042}});
  }
  {
    auto a = u8"🍀";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x1F340}});
  }
  {
    auto a = u8"\uFFFD";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0xFFFD}});
  }
  {
    std::u8string a {u8'\0'};
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0}});
  }
  {
    auto a = u8"\u007F";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x7F}});
  }
  {
    auto a = u8"\u0080";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x80}});
  }
  {
    auto a = u8"\u0081";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x81}});
  }
  {
    auto a = u8"\u07FF";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x7FF}});
  }
  {
    auto a = u8"\u0800";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x800}});
  }
  {
    auto a = u8"\u0801";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x801}});
  }
  {
    auto a = u8"\uFFFF";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0xFFFF}});
  }
  {
    auto a = u8"\U00010000";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x10000}});
  }
  {
    auto a = u8"\U00010001";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x10001}});
  }
  {
    auto a = u8"\U0010FFFF";
    auto b = codepoint (a);
    t.assert_eq (b, std::optional {chino::utf8::codepoint_t {0x10FFFF}});
  }
  {
    // 0-indexedで0バイト目が不正
    for (char8_t i = 0b10000000; i != 0b11111111; i |= i >> 1)
    {
      std::u8string a {i};
      auto b = codepoint (a);
      t.assert_eq (b, std::optional <chino::utf8::codepoint_t> {});
    }
    t.assert_eq (codepoint (std::u8string {0xFF}), std::optional <chino::utf8::codepoint_t> {});
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xC0, 0xAF}};
    auto b = codepoint (a);
    t.assert_eq (b, std::optional <chino::utf8::codepoint_t> {});
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xE0, 0x80, 0xAF}};
    auto b = codepoint (a);
    t.assert_eq (b, std::optional <chino::utf8::codepoint_t> {});
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xF0, 0x80, 0x80, 0xAF}};
    auto b = codepoint (a);
    t.assert_eq (b, std::optional <chino::utf8::codepoint_t> {});
  }
  {
    // 途中で終わっている
    std::u8string a {{0xC3, 0x2F}};
    auto b = codepoint (a);
    t.assert_eq (b, std::optional <chino::utf8::codepoint_t> {});
  }
  {
    // サロゲート用コード値
    std::u8string a {{0xED, 0xA0, 0x80}};
    auto b = codepoint (a);
    t.assert_eq (b, std::optional <chino::utf8::codepoint_t> {});
  }
  {
    // サロゲート用コード値
    std::u8string a {{0xED, 0xBF, 0xBF}};
    auto b = codepoint (a);
    t.assert_eq (b, std::optional <chino::utf8::codepoint_t> {});
  }
}

inline auto test_validate (chino::test::test & t)
{
  using namespace std::literals;
  {
    // valid
    auto a = u8"aαあ🍀\uFFFD\0\u007F\u0080\u0081\u07FF\u0800\u0801\uFFFF\U00010000\U00010001\U0010FFFFinudex emiawp/fea3$A&&$5\t\n\v\a\b\f\\"sv;
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, a.length ());
  }
  {
    // 0-indexedで0バイト目が不正
    for (char8_t i = 0b10000000; i != 0b11111111; i |= i >> 1)
    {
      std::u8string a {i};
      auto b = chino::utf8::validate (a);
      t.assert_eq (b, 0zu);
    }
    t.assert_eq (chino::utf8::validate (std::u8string {0xFF}), 0zu);
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xC0, 0xAF}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 0zu);
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xE0, 0x80, 0xAF}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 0zu);
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xF0, 0x80, 0x80, 0xAF}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 0zu);
  }
  {
    // 途中で終わっている
    std::u8string a {{0xC3, 0x2F}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 0zu);
  }
  {
    // サロゲート用コード値
    std::u8string a = {{0xED, 0xA0, 0x80}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 0zu);
  }
  {
    // サロゲート用コード値
    std::u8string a = {{0xED, 0xBF, 0xBF}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 0zu);
  }
  {
    // 途中まで正常
    std::u8string a = {{0xE3, 0x81, 0x82, 0xFF}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 3zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 9zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 8zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 7zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 6zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 5zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 4zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 3zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 2zu);
  }
  {
    // ascii用高速化処理が正常かどうか
    std::u8string a = {{u8'a', 0xFF, u8'a', u8'a', u8'a', u8'a', u8'a', u8'a', u8'a'}};
    auto b = chino::utf8::validate (a);
    t.assert_eq (b, 1zu);
  }
}

inline auto test_codepoint_to_u8string (chino::test::test & t)
{
  using chino::utf8::codepoint_to_u8string;
  {
    // ascii
    for (char8_t i = 0; i < 0x80; ++ i)
    {
      t.assert_eq (codepoint_to_u8string (i), std::optional {std::u8string {i}});
    }
  }
  t.assert_eq (codepoint_to_u8string (0x80), std::optional {std::u8string {u8"\u0080"}});
  t.assert_eq (codepoint_to_u8string (0x7FF), std::optional {std::u8string {u8"\u07FF"}});
  t.assert_eq (codepoint_to_u8string (0x800), std::optional {std::u8string {u8"\u0800"}});
  t.assert_eq (codepoint_to_u8string (0xFFFF), std::optional {std::u8string {u8"\uFFFF"}});
  t.assert_eq (codepoint_to_u8string (0x10000), std::optional {std::u8string {u8"\U00010000"}});
  t.assert_eq (codepoint_to_u8string (0x10FFFF), std::optional {std::u8string {u8"\U0010FFFF"}});
  t.assert_eq (codepoint_to_u8string (0x110000), std::optional <std::u8string> {});
  t.assert_eq (codepoint_to_u8string (0xD800), std::optional <std::u8string> {});
  t.assert_eq (codepoint_to_u8string (0xDFFF), std::optional <std::u8string> {});
}

auto main () -> int
{
  int res = 0;
  res |= chino::test::add_test (test_utf8_char_width);
  res |= chino::test::add_test (test_is_subsequent);
  res |= chino::test::add_test (test_unsafe_codepoint);
  res |= chino::test::add_test (test_codepoint);
  res |= chino::test::add_test (test_validate);
  res |= chino::test::add_test (test_codepoint_to_u8string);
  return res;
}

