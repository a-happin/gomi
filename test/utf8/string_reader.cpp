#include <chino/utf8/string_reader.hpp>
#include "../test.hpp"

inline auto test_stringreader ()
{
  using chino::utf8::StringReader;
  using namespace std::literals;

  auto reader = StringReader (u8"hoge\n\r\n\rあ\0"sv);
  test::assert_eq (reader.as_str (), u8"hoge\n\r\n\rあ\0"sv);

  test::assert_eq (reader.peek (), U'h');
  test::assert_eq (reader.pos, StringReader::Position {1, 1});
  reader.next ();
  test::assert_eq (reader.peek (), U'o');
  test::assert_eq (reader.pos, StringReader::Position {1, 2});
  reader.next ();
  test::assert_eq (reader.peek (), U'g');
  test::assert_eq (reader.pos, StringReader::Position {1, 3});
  reader.next ();
  test::assert_eq (reader.peek (), U'e');
  test::assert_eq (reader.pos, StringReader::Position {1, 4});
  reader.next ();
  test::assert_eq (reader.peek (), U'\n');
  test::assert_eq (reader.pos, StringReader::Position {1, 5});
  reader.next ();
  test::assert_eq (reader.peek (), U'\r');
  test::assert_eq (reader.pos, StringReader::Position {2, 1});
  reader.next ();
  test::assert_eq (reader.peek (), U'\r');
  test::assert_eq (reader.pos, StringReader::Position {3, 1});
  reader.next ();
  test::assert_eq (reader.peek (), U'あ');
  test::assert_eq (reader.pos, StringReader::Position {4, 1});
  reader.next ();
  test::assert_eq (reader.peek (), U'\0');
  test::assert_eq (reader.pos, StringReader::Position {4, 2});
  reader.next ();
  test::assert_eq (reader.can_read (), false);
  test::assert_eq (reader.pos, StringReader::Position {4, 3});

  test::expect_exception <chino::utf8::invalid_utf8_error> ([] () {
    char8_t invalid_str[] = {0xFF, 0};
    auto invalid_reader = StringReader {invalid_str};
    static_cast <void> (invalid_reader);
  });
}

auto main () -> int
{
  std::ios::sync_with_stdio (false);

  test_stringreader ();
  return test::result ();
}
