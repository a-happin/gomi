#ifndef CHINO_UTF8_HPP
#define CHINO_UTF8_HPP
#include <string>
#include <string_view>
#include <cstring>
#include <chino/optional.hpp>
#include <chino/type_traits/expect_type.hpp>

namespace chino::utf8
{
  namespace detail
  {
    template <typename T>
    inline constexpr auto is_aligned (const void * p) noexcept -> bool
    {
      static_assert ((alignof (T) & (alignof (T) - 1)) == 0, "alignof (T) is not pow2");
      return (reinterpret_cast <std::uintptr_t> (p) & (alignof (T) - 1)) == 0;
    }

    template <char8_t min, char8_t max>
    inline constexpr auto is_in_range (char8_t x) noexcept -> bool
    {
      static_assert (min <= max);
      return min <= x && x <= max;
    }

    inline constexpr uint_fast8_t utf8_char_width [256] =
    {
      // 1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 1
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 2
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 3
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 4
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 5
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 6
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 7
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B
      0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // D
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // E
      4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // F
      // 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0, // F
    };
  }
  using codepoint_t = uint_fast32_t;

  inline constexpr auto utf8_char_width (char8_t c) noexcept -> std::size_t
  {
    return detail::utf8_char_width[c];
  }

  inline constexpr auto is_subsequent (char8_t c) noexcept -> bool
  {
    return static_cast <std::int8_t> (c) < -64;
  }

  // returns byte length of valid utf-8 string: std::size_t
  inline constexpr auto validate (std::u8string_view str) noexcept -> std::size_t
  {
    // 0XXXXXXX 0~7
    // 110YYYYX 10XXXXXX ~11
    // 1110YYYY 10YXXXXX 10XXXXXX ~16
    // 11110YYY 10YYXXXX 10XXXXXX 10XXXXXX ~21
    // 111110YY 10YYYXXX 10XXXXXX 10XXXXXX 10XXXXXX ~26
    // 1111110Y 10YYYYXX 10XXXXXX 10XXXXXX 10XXXXXX 10XXXXXX ~31
    // X = 0 or 1
    // Y = どれか1つは1

    // サロゲートコードポイント
    // U+D800 .. U+DFFF
    // U+D800 = 11101101 10100000 10000000
    // U+DFFF = 11101101 10111111 10111111

    // Unicode
    // U+0000 .. U+10FFFF
    // U+10FFFF = 11110100 10001111 10111111 10111111

    using detail::is_in_range;

    auto begin = str.data ();
    auto end = begin + str.length ();
    for (auto p = begin; p < end; ++ p)
    {
      auto old_p = p;
      switch (utf8_char_width (* p))
      {
        //   vaild utf-8 character => continue;
        // invalid utf-8 character => break;
        case 1:
          // ascii用高速化処理
          // is_alignedの判定は処理時間に影響がほぼなかった(むしろ遅くなってるまである)のでなしにした
          using buf_t = std::size_t;
          while (p + sizeof (buf_t) < end)
          {
            buf_t buf;
            std::memcpy (&buf, p + 1, sizeof (buf_t));
            if ((buf & static_cast <buf_t> (0x8080808080808080u)) == 0)
            {
              p += sizeof (buf_t);
            }
            else
            {
              break;
            }
          }
          continue;
        case 2:
          if (p + 1 < end && is_subsequent (* ++ p)) continue;
          break;
        case 3:
          if (not (p + 2 < end)) break;
          if (* p == 0xE0)
          {
            // 冗長表現をエラーにする
            if (is_in_range <0xA0, 0xBF> (* ++ p));
            else break;
          }
          else if (is_in_range <0xE1, 0xEC> (* p))
          {
            if (is_subsequent (* ++ p));
            else break;
          }
          else if (* p == 0xED)
          {
            // サロゲートコードポイントをエラーにする
            if (is_in_range <0x80, 0x9F> (* ++ p));
            else break;
          }
          else if (is_in_range <0xEE, 0xEF> (* p))
          {
            if (is_subsequent (* ++ p));
            else break;
          }
          else break;
          if (not (is_subsequent (* ++ p))) break;
          continue;
        case 4:
          if (not (p + 3 < end)) break;
          if (* p == 0xF0)
          {
            // 冗長表現をエラーにする
            if (is_in_range <0x90, 0xBF> (* ++ p));
            else break;
          }
          else if (is_in_range <0xF1, 0xF3> (* p))
          {
            if (is_subsequent (* ++ p));
            else break;
          }
          else if (* p == 0xF4)
          {
            // U+10FFFFより大きいコードポイントをエラーにする
            if (is_in_range <0x80, 0x8F> (* ++ p));
            else break;
          }
          else break;
          if (not (is_subsequent (* ++ p))) break;
          if (not (is_subsequent (* ++ p))) break;
          continue;
        default:
          break;
      }
      return static_cast <std::size_t> (old_p - begin);
    }
    return str.length ();
  }

  struct validated_u8string_view
  {
    std::u8string_view str;

    explicit constexpr validated_u8string_view (std::u8string_view str_) noexcept
      : str {std::move (str_)}
    {}
  };

  inline namespace literals
  {
    inline namespace validated_u8string_view_lietals
    {
      inline constexpr auto operator ""_sv (const char8_t * str, std::size_t n) noexcept -> validated_u8string_view
      {
        return validated_u8string_view {std::u8string_view {str, n}};
      }
    }
  }


  inline constexpr auto unsafe_codepoint (std::u8string_view str) noexcept -> codepoint_t
  {
    auto width = utf8_char_width (str[0]);
    codepoint_t c = str[0] & (0b11111111u >> width);
    for (std::remove_cvref_t <decltype (width)> i = 1; i < width; ++ i)
    {
      c <<= 6;
      c |= str[i] & 0b00111111;
    }
    return c;
  }

  inline constexpr auto codepoint (std::u8string_view str) noexcept -> std::optional <codepoint_t>
  {
    if (auto len = utf8_char_width (str[0]); len != 0)
    {
      if (str.length () < len) return std::nullopt;
      codepoint_t c = str[0] & (0b11111111u >> len);
      for (std::remove_cvref_t <decltype (len)> i = 1; i < len; ++ i)
      {
        if (not is_subsequent (str[i])) return std::nullopt;
        c <<= 6;
        c |= str[i] & 0b00111111;
      }
      // 冗長な表現をエラーにする
      constexpr codepoint_t least[] = {0, 0x80, 0x800, 0x10000, 0x200000, 0x400000};
      if (c < least[-- len]) return std::nullopt;
      // サロゲートペア用コード値をエラーにする
      if (0xD800 <= c && c <= 0xDFFF) return std::nullopt;
      return c;
    }
    return std::nullopt;
  }

  inline constexpr auto codepoint (validated_u8string_view str) noexcept -> codepoint_t
  {
    return unsafe_codepoint (str.str);
  }

  inline auto codepoint_to_u8string (codepoint_t val) -> std::optional <std::u8string>
  {
    if (val < 0x80)
    {
      return std::u8string {static_cast <char8_t> (val)};
    }
    if (0xD800 <= val && val <= 0xDFFF) return std::nullopt;
    if (val > 0x10FFFF) return std::nullopt;
    constexpr auto as_size = [] (bool x) constexpr noexcept -> std::size_t { return x ? 1 : 0; };
    auto len = 2zu + as_size (val >= 0x800) + as_size (val >= 0x10000);
    char8_t buffer[4] = {static_cast <char8_t> (0b11111100000000u >> len), 0b10000000, 0b10000000, 0b10000000};
    for (decltype (len) i = 1; i < len; ++ i)
    {
      buffer[len - i] |= static_cast <char8_t> (val & 0b00111111);
      val >>= 6;
    }
    buffer[0] |= static_cast <char8_t> (val);
    return std::u8string {buffer, len};
  }
}
#endif
