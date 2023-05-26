#include <chino/utf8/source.hpp>
#include <iostream>
#include "../test.hpp"

inline auto test_lines ()
{
  {
    std::u8string str {u8""};
    auto lines = chino::utf8::source::lines (str);
    test::assert_eq (lines.size (), 0zu);
  }
  {
    std::u8string str {u8"\n"};
    auto lines = chino::utf8::source::lines (str);
    test::assert_eq (lines.size (), 1zu);
  }
  {
    std::u8string str {u8"\n\r\n"};
    auto lines = chino::utf8::source::lines (str);
    test::assert_eq (lines.size (), 2zu);
  }
}

auto main () -> int
{
  std::ios::sync_with_stdio (false);
  std::cin.tie (nullptr);

  std::u8string_view source {u8"foo"};

  auto ite = chino::utf8::source::SourceReader {source};
  test::assert_eq (ite.peek (), std::optional {U'f'});
  test::assert_eq (ite.next (), std::optional {U'f'});
  test::assert_eq (ite.next (), std::optional {U'o'});
  test::assert_eq (ite.next (), std::optional {U'o'});
  test::assert_eq (ite.next (), std::optional <char32_t> {});

  test_lines ();

  return test::result ();
}
