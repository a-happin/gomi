#ifndef CHINO_UTF8_STRING_READER_HPP
#define CHINO_UTF8_STRING_READER_HPP
#include <stdexcept>
#include <sstream>
#include <chino/utf8.hpp>

namespace chino::utf8
{
  struct invalid_utf8_error : std::runtime_error
  {
    invalid_utf8_error (std::ptrdiff_t pos)
      : std::runtime_error {
        [&] () {
          std::ostringstream ss;
          ss << "UTF-8として不正な文字列です\n" << pos << "バイト目に不明なバイト列を検出しました";
          return std::move (ss).str ();
        } ()
      }
    {
    }
    ~ invalid_utf8_error () noexcept override;
  };
  // TODO: 外に出す
  invalid_utf8_error::~ invalid_utf8_error () noexcept = default;

  struct StringReader
  {
    using pointer_t = const char8_t *;

    pointer_t ite, end;
    struct Position
    {
      std::size_t line, col;
      friend constexpr auto operator == (const Position &, const Position &) noexcept -> bool = default;
      friend constexpr auto operator <=> (const Position &, const Position &) noexcept -> std::strong_ordering = default;
    } position;

    constexpr StringReader () noexcept
      : ite {nullptr}
      , end {nullptr}
      , position {1, 1}
    {
    }

    constexpr StringReader (std::u8string_view str)
      : ite {str.data ()}
      , end {str.data () + str.length ()}
      , position {1, 1}
    {
      if (auto p = chino::utf8::find_invalid (str); p != nullptr)
      {
        throw invalid_utf8_error {str.end () - p + 1};
      }
    }

    constexpr auto as_str () const noexcept
    {
      return std::u8string_view {ite, end};
    }

    constexpr auto can_read () const noexcept
    {
      return ite < end;
    }

    constexpr auto peek () const noexcept
    {
      return chino::utf8::unsafe_codepoint (ite);
    }

    constexpr auto next () noexcept -> decltype (auto)
    {
      if (* ite == u8'\n')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
      }
      else if (* ite == u8'\r')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
        if (can_read () && * ite == u8'\n')
        {
          ++ ite;
        }
      }
      else
      {
        ite += chino::utf8::char_width (* ite);
        ++ position.col;
      }
      return * this;
    }
  };
}

#endif

