#include <chino/utf8.hpp>

#define IUTEST_USE_MAIN
#include <iutest.hpp>

IUTEST (substrTest, valid)
{
  using namespace chino::utf8::literals;
  {
    chino::utf8::valid_u8string_view str {u8"This is a pen"_sv};
    auto ret1 = str.substr_in_bytes (5);
    auto ret2 = str.substr_in_bytes (5, 2);
    IUTEST_EXPECT (ret1 == u8"is a pen"_sv);
    IUTEST_EXPECT (ret2 == u8"is"_sv);
  }
}

IUTEST (substrTest, invalid)
{
  using namespace chino::utf8::literals;
  IUTEST_EXPECT_ANY_THROW(u8"あ"_sv.substr_in_bytes (1));
}

