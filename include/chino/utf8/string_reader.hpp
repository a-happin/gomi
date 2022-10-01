#ifndef CHINO_UTF8_STRING_READER_HPP
#define CHINO_UTF8_STRING_READER_HPP
#include <chino/utf8.hpp>
#include <tuple>
#include <stdexcept>
#include <sstream>

#include <span>

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
      : ptr {nullptr}
      , last {nullptr}
    {}

    constexpr BinaryReader (pointer_t first_, pointer_t last_) noexcept
      : ptr {first_}
      , last {last_}
    {}

    constexpr auto position () const noexcept
    {
      return pos;
    }

    constexpr auto pointer () const noexcept
    {
      return ptr;
    }

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

  struct StringReader
  {
    using pointer_t = const char8_t *;
    struct Position
    {
      std::size_t line, col;
      friend constexpr auto operator == (const Position &, const Position &) noexcept -> bool = default;
      friend constexpr auto operator <=> (const Position &, const Position &) noexcept -> std::strong_ordering = default;
      friend auto operator << (std::ostream & stream, const Position & pos_) -> decltype (auto)
      {
        return stream << "(line = " << pos_.line << ", col = " << pos_.col << ")";
      }
    };

  private:
    pointer_t ptr, end;
    Position pos;

  public:
    constexpr StringReader () noexcept
      : ptr {nullptr}
      , end {nullptr}
      , pos {1, 1}
    {
    }

    template <typename Recover = decltype ([] [[noreturn]] (std::u8string_view valid_str) -> std::u8string_view
    {
      std::ostringstream ss;
      ss << "UTF-8として不正な文字列です。" << (valid_str.size () + 1) << "バイト目に不明なバイト列を検出しました";
      throw invalid_utf8_error {std::move (ss).str ()};
    })>
      requires std::same_as <std::invoke_result_t <Recover, std::u8string_view>, std::u8string_view>
    explicit constexpr StringReader (std::u8string_view str, Recover && recover = {})
      noexcept (noexcept (recover (str)))
      : ptr {str.data ()}
      , end {str.data () + str.length ()}
      , pos {1, 1}
    {
      if (auto p = chino::utf8::find_invalid (str); p != nullptr)
      {
        auto recovered_str = recover (std::u8string_view {str.data (), p});
        std::tie (ptr, end) = std::tuple {recovered_str.data (), recovered_str.data () + recovered_str.length ()};
      }
    }

    constexpr auto position () const noexcept
    {
      return pos;
    }

    constexpr auto pointer () const noexcept
    {
      return ptr;
    }

    constexpr auto as_str () const noexcept
    {
      return std::u8string_view {ptr, end};
    }

    constexpr auto can_read () const noexcept
    {
      return ptr < end;
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
        ++ pos.line;
        pos.col = 1;
      }
      else if (* ptr == u8'\r')
      {
        ++ ptr;
        ++ pos.line;
        pos.col = 1;
        if (can_read () && * ptr == u8'\n')
        {
          ++ ptr;
        }
      }
      else
      {
        ptr += chino::utf8::char_width (* ptr);
        ++ pos.col;
      }
      return * this;
    }
  };
}

#endif

