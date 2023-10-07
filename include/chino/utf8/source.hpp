#ifndef CHINO_UTF8_SOURCE_HPP
#define CHINO_UTF8_SOURCE_HPP
#include <chino/utf8.hpp>
#include <vector>

namespace chino::utf8::source
{
  struct SourcePosition {
    using Self = SourcePosition;
    std::uint32_t line, col;

    friend constexpr auto operator == (const Self &, const Self &) noexcept -> bool = default;
    friend constexpr auto operator <=> (const Self &, const Self &) noexcept -> std::strong_ordering = default;
    template <std::same_as <char> CharT>
    friend auto operator << (std::basic_ostream <CharT> & stream, const Self & self) noexcept -> decltype (auto)
    {
      return stream << "(line = " << (self.line + 1) << ", col = " << (self.col + 1) << ')';
    }
  };

  struct SourceSpan {
    using Self = SourceSpan;
    SourcePosition begin;
    SourcePosition end;
    std::u8string_view str;

    friend constexpr auto operator == (const Self &, const Self &) noexcept -> bool = default;
    template <std::same_as <char> CharT>
    friend auto operator << (std::basic_ostream <CharT> & stream, const Self & self) noexcept -> decltype (auto)
    {
      return stream << self.begin << '~' << self.end << ':' << '`' << std::basic_string_view <CharT> {reinterpret_cast <const CharT *> (self.str.data ()), self.str.length ()} << '`';
    }
  };

  template <typename T>
  struct WithSpan
  {
    using Self = WithSpan;
    SourceSpan span;
    T value;

    // = default;だとエラーになる(は？)
    friend constexpr auto operator == (const Self & lhs, const Self & rhs) noexcept -> bool // = default;
    {
      return lhs.span == rhs.span && lhs.value == rhs.value;
    }
  };
  template <typename T>
  WithSpan (SourceSpan &&, T &&) -> WithSpan <std::remove_cvref_t <T>>;

  struct SourceReader {
    using Self = SourceReader;
    using pointer_t = const char8_t *;

  private:
    pointer_t ptr, last;
    SourcePosition pos {0, 0};

    constexpr auto next_impl () noexcept
    {
        if (* ptr == u8'\n')
        {
          ++ pos.line;
          pos.col = 0;
        }
        else
        {
          ++ pos.col;
        }
        ptr += chino::utf8::char_width (* ptr);
    }

  public:
    constexpr SourceReader () noexcept
      : SourceReader {nullptr, nullptr}
    {}

    constexpr SourceReader (pointer_t first_, pointer_t last_) noexcept
      : ptr {first_}
      , last {last_}
    {}

    explicit constexpr SourceReader (std::u8string_view str) noexcept
      : SourceReader {str.data (), str.data () + str.size ()}
    {}

    /* constexpr SourceIterator (const SourceIterator &) noexcept = default; */
    /* constexpr SourceIterator (SourceIterator &&) noexcept = default; */
    /* constexpr auto operator = (const SourceIterator &) noexcept -> Self & = default; */
    /* constexpr auto operator = (SourceIterator &&) noexcept -> Self & = default; */

    constexpr auto position () const noexcept -> decltype (auto)
    {
      return pos;
    }

    constexpr auto pointer () const noexcept -> decltype (auto)
    {
      return ptr;
    }

    constexpr auto can_read () const noexcept
    {
      return ptr < last;
    }

    constexpr auto peek () const noexcept -> std::optional <char32_t>
    {
      if (can_read ())
      {
        return chino::utf8::unsafe_codepoint (ptr);
      }
      else
      {
        return {};
      }
    }

    constexpr auto as_str () const noexcept
    {
      return std::u8string_view {ptr, last};
    }

    constexpr auto as_str_to (const Self & to) const noexcept
    {
      return std::u8string_view {ptr, to.ptr};
    }

    template <typename Pred> requires requires (Pred pred) {
      {pred (U'0')} -> std::same_as <bool>;
    }
    [[nodiscard]] constexpr auto expects (Pred pred) const noexcept -> bool
    {
      auto c = peek ();
      return c && pred (* c);
    }

    constexpr auto next () noexcept -> std::optional <char32_t>
    {
      if (auto c = peek (); c)
      {
        next_impl ();
        return c;
      }
      else
      {
        return {};
      }
    }

    template <typename Pred> requires requires (Pred pred, char32_t c) {
      {pred (std::move (c))} -> std::same_as <bool>;
    }
    [[nodiscard]] constexpr auto next_if (Pred pred) noexcept -> std::optional <char32_t>
    {
      if (auto c = peek (); c && pred (* c))
      {
        next_impl ();
        return c;
      }
      else
      {
        return {};
      }
    }

    constexpr auto get_span_to (const Self & to) const noexcept
    {
      auto begin = position ();
      auto end = to.position ();
      auto str = as_str_to (to);
      return SourceSpan {begin, end, str};
    }

    constexpr auto get_empty_span () const noexcept
    {
      return get_span_to (* this);
    }

    constexpr auto get_span_just () const noexcept
    {
      std::remove_cvref_t <decltype (* this)> tmp {* this};
      tmp.next ();
      return get_span_to (tmp);
    }

    constexpr auto read_to (Self && to) noexcept
    {
      auto res = get_span_to (to);
      * this = std::move (to);
      return res;
    }

    /* template <typename F> requires requires (F f, Self & ite) { */
    /*   {f (ite)} -> std::same_as <std::optional <typename std::invoke_result_t <F, Self &>::value_type>>; */
    /* } */
    /* constexpr auto read (F f) noexcept -> std::optional <WithSpan <typename std::invoke_result_t <F, Self &>::value_type>> */
    /* { */
    /*   std::remove_cvref_t <decltype (* this)> tmp {* this}; */
    /*   if (auto res_opt = f (tmp)) */
    /*   { */
    /*     auto res = * std::move (res_opt); */
    /*     return WithSpan {read_to (std::move (tmp)), std::move (res)}; */
    /*   } */
    /*   else */
    /*   { */
    /*     return {}; */
    /*   } */
    /* } */

    friend constexpr auto operator == (const Self &, const Self &) noexcept -> bool = default;
  };

  enum ErrorLevel : uint8_t
  {
    Error = 1,
    Warning = 2,
    Info = 3,
    Hint = 4,
  };

  namespace ops {
    template <std::same_as <char> CharT>
    constexpr auto operator << (std::basic_ostream <CharT> & stream, ErrorLevel & level) noexcept -> decltype (auto)
    {
      switch (level)
      {
        case Error:
          stream << "Error";
          break;
        case Warning:
          stream << "Warning";
          break;
        case Info:
          stream << "Info";
          break;
        case Hint:
          stream << "Hint";
          break;
      }
      return stream;
    }
  }
  using ops::operator <<;

  /* template <typename T> */
  /* concept Printable = requires (std::basic_ostream <char> & stream, const T &value) */
  /* { */
  /*   {stream << value} -> std::same_as <std::basic_ostream <char> &>; */
  /* }; */
  /* struct PrinableDummy { */
  /*   friend constexpr auto operator << (std::basic_ostream <char> & stream, const PrinableDummy &) -> std::basic_ostream <char> &; */
  /* }; */

  /* template <typename T> */
  /* concept ErrorHandler = requires (T handler, ErrorLevel level, SourceSpan span, const PrinableDummy & message) */
  /* { */
  /*   {handler (level, span, message)}; */
  /* }; */


  // deprecated in C++23: use std::views::split (str, u8'\n') instead
  inline constexpr auto lines (std::u8string_view str) -> std::vector <std::u8string_view>
  {
    std::vector <std::u8string_view> res;
    auto begin = str.begin ();
    auto ite = begin;
    auto end = str.end ();
    while (ite < end)
    {
      if (* ite == u8'\n' /* || * ite == u8'\r' */)
      {
        res.emplace_back (begin, ite);
        ++ ite;
        /* if (* ite == u8'\n') */
        /* { */
        /*   ++ ite; */
        /* } */
        /* else */
        /* { */
        /*   ++ ite; */
        /*   if (ite < end && * ite == '\n') */
        /*   { */
        /*     ++ ite; */
        /*   } */
        /* } */
        begin = ite;
      }
      else
      {
        ++ ite;
      }
    }
    if (begin != ite)
    {
      res.emplace_back (begin, ite);
    }
    return res;
  }
}
#endif

