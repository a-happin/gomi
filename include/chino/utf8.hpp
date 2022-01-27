#ifndef CHINO_UTF8_HPP
#define CHINO_UTF8_HPP
#include <cstdint>
#include <string_view>
#include <cstring> // std::memcpy
#include <optional>
#include <ostream>
#include <span>

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

  struct valid_u8string_view;


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
    constexpr auto as_size = [] (bool x) constexpr noexcept -> std::size_t { return x ? 1 : 0; };
    auto len = 2zu + as_size (val >= 0x800) + as_size (val >= 0x10000);
    char8_t buffer[4] = {static_cast <char8_t> (0b11111100000000u >> len), 0b10000000, 0b10000000, 0b10000000};
    for (decltype (len) i = 1; i < len; ++ i)
    {
      buffer[len - i] |= static_cast <char8_t> (val & 0b00111111);
      val >>= 6;
    }
    buffer[0] |= static_cast <char8_t> (val);
    for (auto && elem : std::span {buffer, len})
    {
      stream << static_cast <CharT> (elem);
    }
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


  // returns byte length of valid utf-8 string: std::size_t
  inline constexpr auto find_invalid (std::u8string_view str) noexcept -> std::size_t
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
    return std::u8string_view::npos;
  }

  // 使い勝手が微妙に悪い
  struct valid_u8string_view
  {
  private:
    std::u8string_view str;
  public:

    constexpr valid_u8string_view () noexcept
      : str {}
    {}

    explicit constexpr valid_u8string_view (std::u8string_view str_) noexcept
      : str {std::move (str_)}
    {}

    struct codepoint_iterator
    {
      using difference_type = std::ptrdiff_t;
      using value_type = codepoint_t;

    private:
      const char8_t * ptr;

    public:
      constexpr codepoint_iterator () noexcept = default;

      explicit constexpr codepoint_iterator (const char8_t * str_) noexcept
        : ptr {str_}
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

    constexpr auto begin () const noexcept
    {
      return str.begin ();
    }
    constexpr auto end () const noexcept
    {
      return str.end ();
    }
    constexpr auto cbegin () const noexcept
    {
      return str.cbegin ();
    }
    constexpr auto cend () const noexcept
    {
      return str.cend ();
    }
    constexpr auto rbegin () const noexcept
    {
      return str.rbegin ();
    }
    constexpr auto rend () const noexcept
    {
      return str.rend ();
    }
    constexpr auto crbegin () const noexcept
    {
      return str.crbegin ();
    }
    constexpr auto crend () const noexcept
    {
      return str.crend ();
    }
    constexpr auto begin_as_codepoint_iterator () const noexcept
    {
      return codepoint_iterator {str.data ()};
    }
    constexpr auto end_as_codepoint_iterator () const noexcept
    {
      return codepoint_iterator {str.data () + str.length ()};
    }

    constexpr auto size_in_bytes () const noexcept
    {
      return str.size ();
    }
    constexpr auto length_in_bytes () const noexcept
    {
      return str.length ();
    }
    [[deprecated]] constexpr auto length_in_chars () const noexcept
    {
      std::size_t res = 0;
      for (auto ite = begin_as_codepoint_iterator (), ite_end = end_as_codepoint_iterator (); ite < ite_end; ++ ite) ++ res;
      return res;
    }
    [[deprecated]] constexpr auto size_in_chars () const noexcept
    {
      return length_in_chars ();
    }
    constexpr auto max_size () const noexcept
    {
      return str.max_size ();
    }
    constexpr auto empty () const noexcept
    {
      return str.empty ();
    }

    constexpr auto front_as_string () const noexcept
    {
      return str.substr (0, char_width (str[0]));
    }
    constexpr auto front_as_codepoint () const noexcept
    {
      return unsafe_codepoint (str);
    }
    constexpr auto data () const noexcept
    {
      return str.data ();
    }

    constexpr auto remove_prefix_in_bytes (std::size_t n) -> valid_u8string_view &
    {
      if (0 < n && n < str.length () && is_subsequent (str[n]))
      {
        throw std::runtime_error {"invalid_utf8_error"};
      }
      str.remove_prefix (n);
      return * this;
    }
    constexpr auto remove_suffix_in_bytes (std::size_t n) -> valid_u8string_view &
    {
      if (0 < n && n < str.length () && is_subsequent (str[str.length () - n]))
      {
        throw std::runtime_error {"invalid_utf8_error"};
      }
      str.remove_suffix (n);
      return * this;
    }
    constexpr auto remove_prefix_1char () noexcept -> valid_u8string_view &
    {
      str.remove_prefix (char_width (str[0]));
      return * this;
    }
    constexpr auto swap (valid_u8string_view & s) noexcept
    {
      return str.swap (s.str);
    }

    constexpr auto copy (char8_t * s, std::size_t n, std::size_t pos = 0) const
    {
      return str.copy (s, n, pos);
    }
    constexpr auto substr_in_bytes (std::size_t pos = 0, std::size_t n = std::u8string_view::npos) const
    {
      return valid_u8string_view {* this}.remove_prefix_in_bytes (pos).remove_suffix_in_bytes (std::max (str.length () - pos, n) - n);
      // copyof (str).remove_prefix (pos).remove_suffix (str.length () - pos < n ? 0 : str.length () - pos - n)
    }
    constexpr auto starts_with (std::u8string_view str_) const noexcept
    {
      return str.starts_with (str_);
    }
    constexpr auto ends_with (std::u8string_view str_) const noexcept
    {
      return str.ends_with (str_);
    }
    constexpr auto contains (std::u8string_view str_) const noexcept
    {
      return str.contains (str_);
    }
    /* constexpr auto starts_with (std::u32string_view str_) const noexcept */
    /* { */
    /*   for (auto ite = begin_as_codepoint_iterator (), ite_end = end_as_codepoint_iterator (); auto && elem : str_) */
    /*   { */
    /*     if (not (ite < ite_end)) */
    /*     { */
    /*       return false; */
    /*     } */
    /*     if (* ite != elem) */
    /*     { */
    /*       return false; */
    /*     } */
    /*     ++ ite; */
    /*   } */
    /*   return true; */
    /* } */

    friend constexpr auto operator == (const valid_u8string_view &, const valid_u8string_view &) noexcept -> bool = default;
    friend constexpr auto operator <=> (const valid_u8string_view &, const valid_u8string_view &) noexcept -> std::strong_ordering = default;
  };

  inline namespace literals
  {
    inline namespace valid_u8string_view_lietals
    {
      inline constexpr auto operator ""_sv (const char8_t * str, std::size_t n) noexcept -> valid_u8string_view
      {
        return valid_u8string_view {std::u8string_view {str, n}};
      }
    }
  }

  inline constexpr auto validate (std::u8string_view str) noexcept -> std::optional <valid_u8string_view>
  {
    if (auto pos = find_invalid (str); pos != std::u8string_view::npos)
    {
      return std::nullopt;
    }
    return valid_u8string_view {str};
  }

  inline constexpr auto codepoint (valid_u8string_view str) noexcept -> codepoint_t
  {
    return str.front_as_codepoint ();
  }

  struct StringReader
  {
    valid_u8string_view str;
    struct Position
    {
      std::size_t line;
      std::size_t col;
      friend constexpr auto operator == (const Position &, const Position &) noexcept -> bool = default;
      friend constexpr auto operator <=> (const Position &, const Position &) noexcept -> std::strong_ordering = default;
    } position;
    constexpr StringReader (valid_u8string_view str_) noexcept
      : str {std::move (str_)}
      , position {1, 1}
    {}

    constexpr auto is_eof () const noexcept
    {
      return str.empty ();
    }

    constexpr auto can_read () const noexcept
    {
      return ! is_eof ();
    }

    constexpr auto peek_as_string () const noexcept
    {
      return str.front_as_string ();
    }

    constexpr auto peek_as_codepoint () const noexcept
    {
      return str.front_as_codepoint ();
    }

    template <typename F>
    requires requires (F && f, std::u8string_view str_)
    {
      {std::forward <F> (f) (str_)} -> std::same_as <bool>;
    }
    constexpr auto try_peek (F && f) const noexcept -> bool
    {
      return can_read () && f (peek_as_string ());
    }

    template <typename F>
    requires requires (F && f, codepoint_t c)
    {
      {std::forward <F> (f) (c)} -> std::same_as <bool>;
    }
    constexpr auto try_peek (F && f) const noexcept -> bool
    {
      return can_read () && f (peek_as_codepoint ());
    }

    constexpr auto starts_with (std::u8string_view str_) const noexcept -> bool
    {
      return str.starts_with (str_);
    }

    constexpr auto operator ++ () noexcept -> StringReader &
    {
      if (str.data ()[0] == '\n')
      {
        ++ position.line;
        position.col = 1;
        str.remove_prefix_in_bytes (1);
      }
      else if (str.data ()[0] == '\r')
      {
        ++ position.line;
        position.col = 1;
        if (str.length_in_bytes () >= 2 && str.data ()[1] == '\n')
        {
          str.remove_prefix_in_bytes (2);
        }
        else
        {
          str.remove_prefix_in_bytes (1);
        }
      }
      else
      {
        ++ position.col;
        str.remove_prefix_1char ();
      }
      return * this;
    }

    constexpr auto operator ++ (int) noexcept -> StringReader
    {
      std::remove_cvref_t <decltype (* this)> tmp {* this};
      ++ * this;
      return tmp;
    }
  };
}

#endif
