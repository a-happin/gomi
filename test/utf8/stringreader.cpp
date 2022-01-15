#include <chino/utf8.hpp>
#include <chino/test.hpp>

inline auto test_stringreader (chino::test::test & t)
{
  using chino::utf8::StringReader;
  using namespace chino::utf8::literals;
  using namespace std::literals;
  auto reader = StringReader (u8"hoge\n\r\n\rあ\0"_sv);
  t.assert_eq (reader.str, u8"hoge\n\r\n\rあ\0"_sv);

  t.assert_eq (reader.peek_as_string (), u8"h"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'h');
  t.assert_eq (reader.position, StringReader::Position {1, 1});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"o"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'o');
  t.assert_eq (reader.position, StringReader::Position {1, 2});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"g"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'g');
  t.assert_eq (reader.position, StringReader::Position {1, 3});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"e"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'e');
  t.assert_eq (reader.position, StringReader::Position {1, 4});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"\n"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'\n');
  t.assert_eq (reader.position, StringReader::Position {1, 5});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"\r"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'\r');
  t.assert_eq (reader.position, StringReader::Position {2, 1});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"\r"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'\r');
  t.assert_eq (reader.position, StringReader::Position {3, 1});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"あ"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'あ');
  t.assert_eq (reader.position, StringReader::Position {4, 1});
  ++ reader;
  t.assert_eq (reader.peek_as_string (), u8"\0"sv);
  t.assert_eq (reader.peek_as_codepoint (), U'\0');
  t.assert_eq (reader.position, StringReader::Position {4, 2});
  ++ reader;
  t.assert (reader.is_eof ());
  t.assert_eq (reader.position, StringReader::Position {4, 3});
}

auto main () -> int
{
  auto res = 0;
  res |= chino::test::add_test (test_stringreader);
  return res;
}
