#ifndef CHINO_UTF8_HPP
#define CHINO_UTF8_HPP
#include <cstdint>
#include <string_view>
#include <cstring> // std::memcpy
#include <optional>
#include <iosfwd>

namespace chino::utf8
{
  // ********************************
  // ** utility
  // ********************************
  namespace detail
  {
    inline constexpr uint_fast8_t char_width [256] =
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

  inline constexpr auto char_width (char8_t c) noexcept -> std::size_t
  {
    return detail::char_width[c];
  }

  inline constexpr auto is_subsequent (char8_t c) noexcept -> bool
  {
    return static_cast <std::int8_t> (c) < -64;
  }


  // ********************************
  // ** codepoint
  // ********************************
  using codepoint_t = char32_t;

  inline constexpr auto unsafe_codepoint (const char8_t * ptr) noexcept -> codepoint_t
  {
    if (auto width = char_width (* ptr); width != 1)
    {
      codepoint_t c = * ptr & (0b11111111u >> width);
      for (std::remove_cvref_t <decltype (width)> i = 1; i < width; ++ i)
      {
        c <<= 6;
        c |= * ++ ptr & 0b00111111;
      }
      return c;
    }
    else
    {
      return * ptr;
    }
  }

  inline constexpr auto unsafe_codepoint (std::u8string_view str) noexcept -> codepoint_t
  {
    return unsafe_codepoint (str.data ());
  }

  inline constexpr auto codepoint (std::u8string_view str) noexcept -> std::optional <codepoint_t>
  {
    if (auto len = char_width (str[0]); len != 0)
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

  template <typename CharT>
  inline auto print_as_utf8 (std::basic_ostream <CharT> & stream, codepoint_t val) -> std::basic_ostream <CharT> &
  {
    if (val < 0x80)
    {
      return stream << static_cast <CharT> (val);
    }
    if (0xD800 <= val && val <= 0xDFFF) return stream;
    if (val > 0x10FFFF) return stream;
    auto len = 1 + (val >= (1 << 11)) + (val >= (1 << 16)) /* + (val >= (1 << 21)) + (val >= (1 << 26)) */;
    auto mask = 0b1111110000000u >> len;
    len *= 6;
    stream << static_cast <CharT> ((mask | (val >> len)) & 0xFF);
    do
    {
      len -= 6;
      stream << static_cast <CharT> ((0b00111111 & (val >> len)) | 0b10000000);
    } while (len != 0);
    return stream;
  }


  // ********************************
  // ** validate
  // ********************************
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
  }


  // returns pointer to the beginning of the invalid string
  // or nullptr if all is valid
  inline constexpr auto find_invalid (std::u8string_view str) noexcept -> const char8_t *
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
      switch (char_width (* p))
      {
        //   vaild utf-8 character => continue;
        // invalid utf-8 character => break;
        case 1:
          // ascii用高速化処理
          // is_alignedの判定は処理時間に影響がほぼなかった(むしろ遅くなってるまである)のでなしにした
          using buf_t = std::uint64_t;
          while (p + sizeof (buf_t) < end)
          {
            buf_t buf;
            std::memcpy (& buf, p + 1, sizeof (buf_t));
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
          if (++ p < end && is_subsequent (* p)) [[likely]] continue;
          break;
        case 3:
          if (not (p + 2 < end)) [[unlikely]] break;
          if (* p == 0xE0)
          {
            // 冗長表現をエラーにする
            if (not is_in_range <0xA0, 0xBF> (* ++ p)) [[unlikely]] break;
          }
          else if (is_in_range <0xE1, 0xEC> (* p))
          {
            if (not is_subsequent (* ++ p)) [[unlikely]] break;
          }
          else if (* p == 0xED)
          {
            // サロゲートコードポイントをエラーにする
            if (not is_in_range <0x80, 0x9F> (* ++ p)) [[unlikely]] break;
          }
          else if (is_in_range <0xEE, 0xEF> (* p))
          {
            if (not is_subsequent (* ++ p)) [[unlikely]] break;
          }
          else [[unlikely]] break;
          if (not is_subsequent (* ++ p)) break;
          continue;
        case 4:
          if (not (p + 3 < end)) break;
          if (* p == 0xF0)
          {
            // 冗長表現をエラーにする
            if (not is_in_range <0x90, 0xBF> (* ++ p)) [[unlikely]] break;
          }
          else if (is_in_range <0xF1, 0xF3> (* p))
          {
            if (not is_subsequent (* ++ p)) [[unlikely]] break;
          }
          else if (* p == 0xF4)
          {
            // U+10FFFFより大きいコードポイントをエラーにする
            if (not is_in_range <0x80, 0x8F> (* ++ p)) [[unlikely]] break;
          }
          else [[unlikely]] break;
          if (not is_subsequent (* ++ p)) [[unlikely]] break;
          if (not is_subsequent (* ++ p)) [[unlikely]] break;
          continue;
        default:
          break;
      }
      return old_p;
    }
    return nullptr;
  }

  struct codepoint_iterator
  {
    using difference_type = std::ptrdiff_t;
    using value_type = codepoint_t;

  private:
    const char8_t * ptr;

  public:
    constexpr codepoint_iterator () noexcept = default;

    explicit constexpr codepoint_iterator (const char8_t * str) noexcept
      : ptr {str}
    {}

  public:
    constexpr auto operator * () const noexcept -> codepoint_t
    {
      return unsafe_codepoint (ptr);
    }

    constexpr auto operator ++ () noexcept -> codepoint_iterator &
    {
      ptr += char_width (ptr[0]);
      return * this;
    }

    constexpr auto operator ++ (int) noexcept -> codepoint_iterator
    {
      std::remove_cvref_t <decltype (* this)> tmp {* this};
      ++ * this;
      return tmp;
    }

    friend constexpr auto operator == (const codepoint_iterator & lhs, const codepoint_iterator & rhs) noexcept -> bool = default;
    friend constexpr auto operator <=> (const codepoint_iterator & lhs, const codepoint_iterator & rhs) noexcept -> std::strong_ordering
    {
      return lhs.ptr <=> rhs.ptr;
    }

    explicit constexpr operator const char8_t * () const noexcept
    {
      return ptr;
    }
  };
  static_assert (std::forward_iterator <codepoint_iterator>);
}

#endif
