#include <sstream>
#include <chino/utf8.hpp>
#include "../test.hpp"

inline auto test_unsafe_codepoint () noexcept
{
  using chino::utf8::unsafe_codepoint;
  using chino::utf8::codepoint_t;
  using namespace std::literals;
  {
    auto a = u8"a";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, U'a');
  }
  {
    auto a = u8"α";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, U'α');
  }
  {
    auto a = u8"あ";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, U'あ');
  }
  {
    auto a = u8"🍀";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, U'🍀');
  }
  {
    auto a = u8"\uFFFD";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0xFFFD});
  }
  {
    std::u8string a {u8'\0'};
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0});
  }
  {
    auto a = u8"\u007F";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x7F});
  }
  {
    auto a = u8"\u0080";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x80});
  }
  {
    auto a = u8"\u0081";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x81});
  }
  {
    auto a = u8"\u07FF";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x7FF});
  }
  {
    auto a = u8"\u0800";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x800});
  }
  {
    auto a = u8"\u0801";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x801});
  }
  {
    auto a = u8"\uFFFF";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0xFFFF});
  }
  {
    auto a = u8"\U00010000";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x10000});
  }
  {
    auto a = u8"\U00010001";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x10001});
  }
  {
    auto a = u8"\U0010FFFF";
    auto b = unsafe_codepoint (a);
    test::assert_eq (b, codepoint_t {0x10FFFF});
  }
}

inline auto test_codepoint () noexcept
{
  using chino::utf8::codepoint;
  using chino::utf8::codepoint_t;
  using namespace std::literals;
  {
    auto a = u8"a";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {u8'a'}});
  }
  {
    auto a = u8"α";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x3B1}});
  }
  {
    auto a = u8"あ";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x3042}});
  }
  {
    auto a = u8"🍀";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x1F340}});
  }
  {
    auto a = u8"\uFFFD";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0xFFFD}});
  }
  {
    std::u8string a {u8'\0'};
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0}});
  }
  {
    auto a = u8"\u007F";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x7F}});
  }
  {
    auto a = u8"\u0080";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x80}});
  }
  {
    auto a = u8"\u0081";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x81}});
  }
  {
    auto a = u8"\u07FF";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x7FF}});
  }
  {
    auto a = u8"\u0800";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x800}});
  }
  {
    auto a = u8"\u0801";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x801}});
  }
  {
    auto a = u8"\uFFFF";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0xFFFF}});
  }
  {
    auto a = u8"\U00010000";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x10000}});
  }
  {
    auto a = u8"\U00010001";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x10001}});
  }
  {
    auto a = u8"\U0010FFFF";
    auto b = codepoint (a);
    test::assert_eq (b, std::optional {codepoint_t {0x10FFFF}});
  }
  {
    // 0-indexedで0バイト目が不正
    for (char8_t i = 0b10000000; i != 0b11111111; i |= i >> 1)
    {
      std::u8string a {i};
      auto b = codepoint (a);
      test::assert_eq (b, std::optional <codepoint_t> {});
    }
    test::assert_eq (codepoint (std::u8string {0xFF}), std::optional <codepoint_t> {});
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xC0, 0xAF}};
    auto b = codepoint (a);
    test::assert_eq (b, std::optional <codepoint_t> {});
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xE0, 0x80, 0xAF}};
    auto b = codepoint (a);
    test::assert_eq (b, std::optional <codepoint_t> {});
  }
  {
    // u8'/'の冗長表現
    std::u8string a {{0xF0, 0x80, 0x80, 0xAF}};
    auto b = codepoint (a);
    test::assert_eq (b, std::optional <codepoint_t> {});
  }
  {
    // 途中で終わっている
    std::u8string a {{0xC3, 0x2F}};
    auto b = codepoint (a);
    test::assert_eq (b, std::optional <codepoint_t> {});
  }
  {
    // サロゲート用コード値
    std::u8string a {{0xED, 0xA0, 0x80}};
    auto b = codepoint (a);
    test::assert_eq (b, std::optional <codepoint_t> {});
  }
  {
    // サロゲート用コード値
    std::u8string a {{0xED, 0xBF, 0xBF}};
    auto b = codepoint (a);
    test::assert_eq (b, std::optional <codepoint_t> {});
  }
}

inline auto test_print_as_utf8 () noexcept
{
  using chino::utf8::print_as_utf8;
  {
    // ascii
    for (char8_t i = 0; i < 0x80; ++ i)
    {
      std::basic_ostringstream <char8_t> stream;
      print_as_utf8 (stream, i);
      test::assert_eq (stream.str (), std::u8string {i});
    }
  }
  {
    std::basic_ostringstream <char8_t> stream;
    print_as_utf8 (stream, 0x80);
    print_as_utf8 (stream, 0x7FF);
    print_as_utf8 (stream, 0x800);
    print_as_utf8 (stream, 0xFFFF);
    print_as_utf8 (stream, 0x10000);
    print_as_utf8 (stream, 0x10FFFF);
    test::assert_eq (stream.str (), std::u8string {u8"\u0080\u07FF\u0800\uFFFF\U00010000\U0010FFFF"});
  }
  {
    std::basic_ostringstream <char8_t> stream;
    print_as_utf8 (stream, 0x110000);
    print_as_utf8 (stream, 0xD800);
    print_as_utf8 (stream, 0xDFFF);
    test::assert_eq (stream.str (), std::u8string {u8""});
  }
}

auto main () -> int
{
  std::ios::sync_with_stdio (false);

  test_unsafe_codepoint ();
  test_codepoint ();
  test_print_as_utf8 ();
  return test::result ();
}
