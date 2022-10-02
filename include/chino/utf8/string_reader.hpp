#ifndef CHINO_UTF8_STRING_READER_HPP
#define CHINO_UTF8_STRING_READER_HPP
#include <chino/utf8.hpp>
#include <chino/char_utils.hpp>
#include <tuple>
#include <stdexcept>
#include <sstream>

namespace chino::utf8
{
  struct invalid_utf8_error : std::runtime_error
  {
    using runtime_error::runtime_error;
    ~ invalid_utf8_error () noexcept override;
  };
  // TODO: 外に出す
  invalid_utf8_error::~ invalid_utf8_error () noexcept = default;

  struct BinaryReader
  {
    using pointer_t = const std::byte *;
    using Position = std::size_t;

  private:
    pointer_t ptr, last;
    Position pos = 1zu;

  public:
    constexpr BinaryReader () noexcept
      : BinaryReader {nullptr, nullptr}
    {}

    constexpr BinaryReader (pointer_t first_, pointer_t last_) noexcept
      : ptr {first_}
      , last {last_}
    {}

    constexpr auto position () const noexcept -> decltype (auto)
    {
      return pos;
    }

    constexpr auto pointer () const noexcept
    {
      return ptr;
    }

    /* constexpr auto passed_span () const noexcept */
    /* { */
    /*   return span.subspan (0, pos); */
    /* } */

    /* constexpr auto remaining_span () const noexcept */
    /* { */
    /*   return span.subspan (pos); */
    /* } */

    constexpr auto as_span () const noexcept
    {
      return std::span {ptr, last};
    }

    constexpr auto can_read () const noexcept
    {
      return ptr < last;
    }

    constexpr auto peek () const noexcept -> decltype (auto)
    {
      return * ptr;
    }

    constexpr auto next () noexcept -> decltype (auto)
    {
      ++ ptr;
      ++ pos;
      return * this;
    }
  };

  struct UnsafeUTF8StringReader
  {
    using pointer_t = const char8_t *;
    struct Position
    {
      std::size_t pos, line, col;
      friend constexpr auto operator == (const Position & lhs, const Position & rhs) noexcept -> bool
      {
        return lhs.pos == rhs.pos;
      }
      friend constexpr auto operator <=> (const Position & lhs, const Position & rhs) noexcept -> std::strong_ordering
      {
        return lhs.pos <=> rhs.pos;
      }
      friend auto operator << (std::ostream & stream, const Position & pos_) -> std::ostream &
      {
        return stream << "(pos = " << pos_.pos << ", line = " << pos_.line << ", col = " << pos_.col << ")";
      }
    };

  private:
    pointer_t ptr, last;
    Position pos {0, 1, 1};

  public:
    constexpr UnsafeUTF8StringReader () noexcept
      : UnsafeUTF8StringReader {nullptr, nullptr}
    {
    }

    constexpr UnsafeUTF8StringReader (pointer_t first_, pointer_t last_) noexcept
      : ptr {first_}
      , last {last_}
    {
    }

    explicit constexpr UnsafeUTF8StringReader (std::u8string_view str) noexcept
      : UnsafeUTF8StringReader {str.data (), str.data () + str.size ()}
    {
    }

    constexpr auto position () const noexcept -> decltype (auto)
    {
      return pos;
    }

    constexpr auto pointer () const noexcept
    {
      return ptr;
    }

    /* constexpr auto passed_string () const noexcept */
    /* { */
      /* return std::u8string_view {first, first + pos.pos}; */
    /* } */

    /* constexpr auto remaining_string () const noexcept */
    /* { */
    /*   return std::u8string_view {ptr, last}; */
    /* } */

    constexpr auto as_str () const noexcept
    {
      return std::u8string_view {ptr, last};
    }

    constexpr auto can_read () const noexcept
    {
      return ptr < last;
    }

    constexpr auto peek () const noexcept
    {
      return chino::utf8::unsafe_codepoint (ptr);
    }

    constexpr auto next () noexcept -> decltype (auto)
    {
      if (* ptr == u8'\n')
      {
        ++ ptr;
        ++ pos.pos;
        ++ pos.line;
        pos.col = 1;
      }
      else if (* ptr == u8'\r')
      {
        ++ ptr;
        ++ pos.pos;
        ++ pos.line;
        pos.col = 1;
        if (can_read () && * ptr == u8'\n')
        {
          ++ ptr;
          ++ pos.pos;
        }
      }
      else
      {
        auto d = chino::utf8::char_width (* ptr);
        ptr += d;
        pos.pos += d;
        ++ pos.col;
      }
      return * this;
    }
  };

  struct UTF8StringReader : UnsafeUTF8StringReader
  {
  public:
    constexpr UTF8StringReader () noexcept
      : UnsafeUTF8StringReader {}
    {}

    constexpr UTF8StringReader (pointer_t first, pointer_t last)
      : UTF8StringReader {std::u8string_view {first, last}}
    {}

    explicit constexpr UTF8StringReader (std::u8string_view str)
      : UnsafeUTF8StringReader {str}
    {
      if (auto p = chino::utf8::find_invalid (str); p != nullptr)
      {
        constexpr auto raise = [] [[noreturn]] (std::u8string_view valid_str)
        {
          std::ostringstream ss;
          ss << "UTF-8として不正な文字列です。" << (valid_str.size () + 1) << "バイト目に不明なバイト列を検出しました";
          throw invalid_utf8_error {std::move (ss).str ()};
        };
        raise (std::u8string_view {str.data (), p});
      }
    }
  };
}

#endif

