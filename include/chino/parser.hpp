#ifndef CHINO_PARSER_HPP
#define CHINO_PARSER_HPP
#include <chino/utf8.hpp>
#include <chino/char_utils.hpp>
#include <tuple>
#include <variant>
#include <vector>

namespace chino::parser
{
  struct never
  {
    constexpr never () noexcept = delete;
  };

  struct StringReader
  {
    chino::utf8::codepoint_iterator ite, end;
    struct Position
    {
      std::size_t line;
      std::size_t col;
    } position;

    constexpr StringReader () noexcept
      : ite {}
      , end {}
      , position {1, 1}
    {}

    constexpr StringReader (std::u8string_view str) noexcept
      : ite {str.data ()}
      , end {str.data () + str.length ()}
      , position {1, 1}
    {}

    inline constexpr auto as_str () const noexcept
    {
      return std::u8string_view {static_cast <const char8_t *> (ite), static_cast <const char8_t *> (end)};
    }

    inline constexpr auto can_read () const noexcept
    {
      return ite < end;
    }

    inline constexpr auto peek () const noexcept
    {
      return * ite;
    }

    inline constexpr auto next () noexcept -> decltype (auto)
    {
      if (* static_cast <const char8_t *> (ite) == u8'\n')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
      }
      else if (* static_cast <const char8_t *> (ite) == u8'\r')
      {
        ++ ite;
        ++ position.line;
        position.col = 1;
        if (can_read () && * static_cast <const char8_t *> (ite) == u8'\n')
        {
          ++ ite;
        }
      }
      else
      {
        ++ ite;
        ++ position.col;
      }
      return * this;
    }
  };

  struct ParserInput
  {
    StringReader reader;
    std::vector <std::string> errors;

    constexpr ParserInput (std::u8string_view source) noexcept
      : reader {source}
      , errors {}
    {}
  };

  namespace result
  {
    template <typename T>
    struct success
    {
      T value;
    };

    template <typename T>
    success (T &&) -> success <std::remove_cvref_t <T>>;

    template <typename T>
    struct failure
    {
      T value;
    };

    template <typename T>
    failure (T &&) -> failure <std::remove_cvref_t <T>>;

    template <typename T, typename E>
    using result_t = std::variant <std::monostate, success <T>, failure <E>>;

    namespace result_traits
    {
      template <typename>
      struct result_traits;

      template <typename T, typename E>
      struct result_traits <result_t <T, E>>
      {
        using success_type = T;
        using failure_type = E;
      };

      template <typename R>
      using success_type = typename result_traits <std::remove_cvref_t <R>>::success_type;

      template <typename R>
      using failure_type = typename result_traits <std::remove_cvref_t <R>>::failure_type;
    }

    template <typename T>
    concept Result = requires
    {
      typename result_traits::success_type <T>;
      typename result_traits::failure_type <T>;
    };

    template <Result R>
    inline constexpr auto is_success (const R & result) noexcept
    {
      return std::holds_alternative <success <result_traits::success_type <R>>> (result);
    }

    template <Result R>
    inline constexpr auto is_failure (const R & result) noexcept
    {
      return std::holds_alternative <failure <result_traits::failure_type <R>>> (result);
    }

    template <Result R>
    inline constexpr auto as_success (R && result) noexcept -> decltype (auto)
    {
      return std::get <success <result_traits::success_type <R>>> (std::forward <R> (result));
    }

    template <Result R>
    inline constexpr auto as_failure (R && result) noexcept -> decltype (auto)
    {
      return std::get <failure <result_traits::failure_type <R>>> (std::forward <R> (result));
    }

    template <Result R>
    inline constexpr auto get_success (R && result) noexcept -> decltype (auto)
    {
      return (as_success (std::forward <R> (result)).value);
    }

    template <Result R>
    inline constexpr auto get_failure (R && result) noexcept -> decltype (auto)
    {
      return (as_failure (std::forward <R> (result)).value);
    }

    template <Result R, typename F>
    inline constexpr auto map (R && result, F && f) noexcept -> result_t <std::remove_cvref_t <decltype (f (get_success (std::forward <R> (result))))>, result_traits::failure_type <R>>
    {
      if (is_success (result))
      {
        return success {f (get_success (std::forward <R> (result)))};
      }
      else if (is_failure (result))
      {
        return as_failure (std::forward <R> (result));
      }
      else
      {
        return {};
      }
    }

    template <Result R, typename F>
    inline constexpr auto and_then (R && result, F && f) noexcept -> std::remove_cvref_t <decltype (f (get_success (std::forward <R> (result))))>
    {
      if (is_success (result))
      {
        return f (get_success (std::forward <R> (result)));
      }
      else if (is_failure (result))
      {
        return as_failure (std::forward <R> (result));
      }
      else
      {
        return {};
      }
    }

    template <Result R, typename F>
    inline constexpr auto catch_error (R && result, F && f) noexcept -> std::remove_cvref_t <decltype (f (get_failure (std::forward <R> (result))))>
    {
      if (is_success (result))
      {
        return as_success (std::forward <R> (result));
      }
      else if (is_failure (result))
      {
        return f (get_failure (std::forward <R> (result)));
      }
      else
      {
        return {};
      }
    }
  }

  template <typename P>
  concept Parser = requires (P p, ParserInput & input)
  {
    {p (input)} -> result::Result;
  };

  template <typename P>
  using ParserResultT = result::result_traits::success_type <std::invoke_result_t <P, ParserInput &>>;

  template <typename P>
  using ParserResultE = result::result_traits::failure_type <std::invoke_result_t <P, ParserInput &>>;


  // --------------------------------
  //  parser combinators
  // --------------------------------

  // map: (Parser <T>, T -> U) -> Parser <U>
  inline constexpr auto map = []
  <Parser P, typename F>
  requires requires (P p, F f, ParserInput & input)
  {
    {result::map (p (input), std::move (f))};
  }
  (P p, F f) constexpr noexcept {
    return [p = std::move (p), f = std::move (f)] (ParserInput & input) constexpr noexcept {
      return result::map (p (input), std::move (f));
    };
  };


  // flat_map: (Parser <T>, T -> Result <U>) -> Parser <U>
  inline constexpr auto flat_map = []
  <typename P, typename F>
  requires requires (P p, F f, ParserInput & input)
  {
    {result::and_then (p (input), std::move (f))} -> result::Result;
  }
  (P p, F f) constexpr noexcept {
    return [p = std::move (p), f = std::move (f)] (ParserInput & input) constexpr noexcept {
      return result::and_then (p (input), std::move (f));
    };
  };


  // failureのflat_map
  // recover: (Parser <T>, std::string -> Result <U>) -> Parser <U>
  inline constexpr auto recover = []
  <typename P, typename F>
  requires requires (P p, F f, ParserInput & input)
  {
    {result::catch_error (p (input), std::move (f))} -> result::Result;
  }
  (P p, F f) constexpr noexcept {
    return [p = std::move (p), f = std::move (f)] (ParserInput & input) constexpr noexcept {
      return result::catch_error (p (input), std::move (f));
    };
  };


  namespace detail
  {
    template <typename, typename ...>
    struct make_variant;

    template <typename T, typename ... Ts, typename ... Us>
    struct make_variant <std::variant <T, Ts ...>, Us ...>
    {
      using type = typename std::conditional_t <(std::is_same_v <T, never> || ... || std::is_same_v <T, Us>), make_variant <std::variant <Ts ...>, Us ...>, make_variant <std::variant <Ts ...>, Us ..., T>>::type;
    };

    template <typename U, typename ... Us>
    struct make_variant <std::variant <>, U, Us ...>
    {
      using type = std::variant <U, Us ...>;
    };

    template <typename U>
    struct make_variant <std::variant <>, U>
    {
      using type = U;
    };

    template <>
    struct make_variant <std::variant <>>
    {
      using type = never;
    };
  }
  template <typename ... Ts>
  using make_variant_t = typename detail::make_variant <std::variant <Ts ...>>::type;
  static_assert (std::is_same_v <std::variant <int, double>, make_variant_t <int, double, double, int>>);
  static_assert (std::is_same_v <int, make_variant_t <int, int>>);
  static_assert (std::is_same_v <int, make_variant_t <int, int, never>>);
  static_assert (std::is_same_v <never, make_variant_t <never>>);


  // and_: (Parser <Ts> ...) -> Parser <std::tuple <ParserResultT <Ts> ...>>
  template <typename E, typename ... Ts, typename P, typename ... Ps>
  inline constexpr auto and_impl (ParserInput & input, std::tuple <Ts ...> && t, P && p, Ps && ... ps) noexcept -> result::result_t <std::tuple <Ts ..., ParserResultT <P>, ParserResultT <Ps> ...>, E>
  {
    auto res = std::forward <P> (p) (input);
    if (is_success (res))
    {
      if constexpr (sizeof ... (Ps) == 0)
      {
        return result::success {std::tuple_cat (std::move (t), std::make_tuple (get_success (std::move (res))))};
      }
      else
      {
        return and_impl <E> (input, std::tuple_cat (std::move (t), std::make_tuple (get_success (std::move (res)))), std::forward <Ps> (ps) ...);
      }
    }
    else if (is_failure (res))
    {
      return result::failure <E> {get_failure (std::move (res))};
    }
    else
    {
      return {};
    }
  }
  inline constexpr auto and_ = [] <typename ... Ps> (Ps ... ps) constexpr noexcept {
    return [... ps = std::move (ps)] (ParserInput & input) constexpr noexcept {
      return and_impl <make_variant_t <ParserResultE <Ps> ...>> (input, std::tuple <> {}, std::move (ps) ...);
    };
  };


  // or_: (Parser <Ts> ...) -> Parser <make_variant_t <std::variant <ParserResultT <Ts> ...>>>
  template <typename T, typename E, typename P, typename ... Ps>
  inline constexpr auto or_impl (ParserInput & input, P && p, Ps && ... ps) noexcept -> result::result_t <T, E>
  {
    auto backup = input.reader;
    auto res = std::forward <P> (p) (input);
    if (is_success (res))
    {
      return result::success <T> {get_success (std::move (res))};
    }
    else if (is_failure (res))
    {
      return result::failure <E> {get_failure (std::move (res))};
    }
    else
    {
      if constexpr (sizeof ... (Ps) == 0)
      {
        return {};
      }
      else
      {
        input.reader = backup;
        return or_impl <T, E> (input, std::forward <Ps> (ps) ...);
      }
    }
  }
  inline constexpr auto or_ = [] <typename ... Ps> (Ps ... ps) constexpr noexcept {
    return [... ps = std::move (ps)] (ParserInput & input) constexpr noexcept {
      return or_impl <make_variant_t <ParserResultT <Ps> ...>, make_variant_t <ParserResultE <Ps> ...>> (input, std::move (ps) ...);
    };
  };


  // --------------------------------
  // example parsers
  // --------------------------------

  inline constexpr auto epsilon = [] (ParserInput & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
  {
    return result::success {input.reader.as_str ().substr (0, 0)};
  };


  inline constexpr auto eof = [] (ParserInput & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
  {
    if (input.reader.can_read ())
    {
      return {};
    }
    else
    {
      return result::success {input.reader.as_str ().substr (0, 0)};
    }
  };


  inline constexpr auto raise = [] (std::string_view message) constexpr noexcept -> result::result_t <never, std::string>
  {
    return result::failure {std::string {message}};
  };


  inline constexpr auto char_ = [] (chino::utf8::codepoint_t c) constexpr noexcept
  {
    return [c = std::move (c)] (ParserInput & input) constexpr noexcept -> result::result_t <chino::utf8::codepoint_t, never> {
      if (input.reader.can_read () && input.reader.peek () == c)
      {
        input.reader.next ();
        return result::success {c};
      }
      else
      {
        return {};
      }
    };
  };


  inline constexpr auto char_if = [] <typename F>
  requires requires (F f, chino::utf8::codepoint_t c)
  {
    {f (c)} -> std::same_as <bool>;
  } (F f) constexpr noexcept {
    return [f = std::move (f)] (ParserInput & input) constexpr noexcept -> result::result_t <chino::utf8::codepoint_t, never>
    {
      if (input.reader.can_read ())
      {
        if (auto c = input.reader.peek (); f (c))
        {
          input.reader.next ();
          return result::success {c};
        }
      }
      return {};
    };
  };


  inline constexpr auto string = [] (std::u8string_view str) constexpr noexcept
  {
    return [str = std::move (str)] (ParserInput & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      if (auto substr = input.reader.as_str ().substr (0, str.length ()); substr == str)
      {
        auto end = chino::utf8::codepoint_iterator {static_cast <const char8_t *> (input.reader.ite) + substr.length ()};
        while (input.reader.ite < end)
        {
          input.reader.next ();
        }
        return result::success {substr};
      }
      else
      {
        return {};
      }
    };
  };


  inline constexpr auto keyword = [] (std::u8string_view str) constexpr noexcept
  {
    return [str = std::move (str)] (ParserInput & input) constexpr noexcept -> result::result_t <std::u8string_view, never>
    {
      if (auto substr = input.reader.as_str ().substr (0, str.length ()); substr == str)
      {
        auto end = chino::utf8::codepoint_iterator {static_cast <const char8_t *> (input.reader.ite) + substr.length ()};
        if (input.reader.end <= end || not chino::char_utils::is_word (* end))
        {
          while (input.reader.ite < end)
          {
            input.reader.next ();
          }
          return result::success {substr};
        }
      }
      return {};
    };
  };
}
#endif

