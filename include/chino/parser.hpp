#ifndef CHINO_PARSER_HPP
#define CHINO_PARSER_HPP
#include <chino/parser/result.hpp>
#include <chino/overload.hpp>
#include <tuple>
#include <vector>

namespace chino::parser
{
  template <typename P, typename I>
  using ParserResultT = result::success_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  using ParserResultE = result::failure_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  concept Parser = result::Result <std::invoke_result_t <P, I &>>;

  // --------------------------------
  //  parser combinators
  // --------------------------------
  #if defined (__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpadded"
  #endif

  /* template <typename T> */
  /* concept is_not_void = not std::same_as <T, void>; */

  // map: (I -> Result <T, E>, T -> U) -> I -> Result <U, E>
  template <typename I>
  inline constexpr auto map = [] <Parser <I> P, typename F> (P p, F f) constexpr noexcept
  {
    static_assert (requires {typename std::invoke_result_t <F, ParserResultT <P, I>>;});
    return [p = std::move (p), f = std::move (f)] (I & input) constexpr noexcept
    {
      return result::map (std::move (p) (input), std::move (f));
    };
  };


  // flat_map: (I -> Result <T, E>, T -> Result <U, E2>) -> I -> Result <U, E | E2>
  template <typename I>
  inline constexpr auto flat_map = [] <Parser <I> P, typename F> (P p, F f) constexpr noexcept
  {
    /* using R = std::invoke_result_t <F, ParserResultT <P, I>, I &>; */
    static_assert (result::Result <std::invoke_result_t <F, ParserResultT <P, I>, I &>>);
    /* using T = result::success_type <R>; */
    /* using E = make_variant <ParserResultE <P, I>, result::failure_type <R>>; */
    return [p = std::move (p), f = std::move (f)] (I & input) constexpr noexcept
    {
      return result::and_then (std::move (p) (input), [f = std::move (f), &input] <typename R> (R && res) constexpr noexcept { return std::move (f) (std::forward <R> (res), input); });
    };
  };


  // failureのflat_map
  // catch_error: (I -> Result <T, E>, E -> Result <U, E2>) -> I -> Result <T | U, E2>
  template <typename I>
  inline constexpr auto catch_error = [] <Parser <I> P, typename F> (P p, F f) constexpr noexcept
  {
    static_assert (result::Result <std::invoke_result_t <F, ParserResultT <P, I>, I &>>);
    return [p = std::move (p), f = std::move (f)] (I & input) constexpr noexcept
    {
      return result::catch_error (std::move (p) (input), [f = std::move (f), &input] <typename R> (R && res) constexpr noexcept { return std::move (f) (std::forward <R> (res), input); });
    };
  };


  // and_: ((I -> Result <Ts, Es>) ...) -> I -> Result <std::tuple <Ts ...>, make_variant <Es ...>>
  template <typename I>
  inline constexpr auto and_ = [] <Parser <I> ... Ps> (Ps ... ps) constexpr noexcept
  {
    using T = std::tuple <ParserResultT <Ps, I> ...>;
    using E = make_variant <ParserResultE <Ps, I> ...>;
    return [... ps = std::move (ps)] (I & input) constexpr noexcept -> result::result <T, E>
    {
      constexpr auto impl = overload
      {
        [] <typename U> (auto &&, I &, U && value) constexpr noexcept -> result::result <T, E>
        {
          return result::success <T> {std::forward <U> (value)};
        },
        [] <typename U, typename P, typename ... Rest> (auto & self, I & i, U && value, P && p, Rest && ... rest) constexpr noexcept -> result::result <T, E>
        {
          if (auto res = std::forward <P> (p) (i); is_success (res))
          {
            return self (self, i, std::tuple_cat (std::forward <U> (value), std::make_tuple (get_success (std::move (res)))), std::forward <Rest> (rest) ...);
          }
          else if (is_failure (res))
          {
            return result::failure <E> {get_failure (std::move (res))};
          }
          else
          {
            return {};
          }
        },
      };
      return impl (impl, input, std::tuple <> {}, std::move (ps) ...);
    };
  };


  // and_with_skip: ((I -> Result <unknown, unknown>), (I -> Result <Ts, Es>) ...) -> I -> Result <std::tuple <Ts ...>, make_variant <Es ...>>
  template <typename I>
  inline constexpr auto and_with_skip = [] <Parser <I> Skip, Parser <I> ... Ps> (Skip skip, Ps ... ps) constexpr noexcept
  {
    using T = std::tuple <ParserResultT <Ps, I> ...>;
    using E = make_variant <ParserResultE <Ps, I> ...>;
    return [skip = std::move (skip), ... ps = std::move (ps)] (I & input) constexpr noexcept -> result::result <T, E>
    {
      constexpr auto impl = overload
      {
        [] <typename U> (auto &&, I &, U && value, auto &&) constexpr noexcept -> result::result <T, E>
        {
          return result::success <T> {std::forward <U> (value)};
        },
        [] <typename U, typename P, typename ... Rest> (auto & self, I & i, U && value, auto && skip_, P && p, Rest && ... rest) constexpr noexcept -> result::result <T, E>
        {
          skip_ (i);
          if (auto res = std::forward <P> (p) (i); is_success (res))
          {
            return self (self, i, std::tuple_cat (std::forward <U> (value), std::make_tuple (get_success (std::move (res)))), skip_, std::forward <Rest> (rest) ...);
          }
          else if (is_failure (res))
          {
            return result::failure <E> {get_failure (std::move (res))};
          }
          else
          {
            return {};
          }
        },
      };
      return impl (impl, input, std::tuple <> {}, std::move (skip), std::move (ps) ...);
    };
  };


  // or_: ((I -> Ts) ...) -> I -> make_variant <Ts ...>
  template <std::copyable I>
  inline constexpr auto or_ = [] <Parser <I> ... Ps> (Ps ... ps) constexpr noexcept
  {
    using T = make_variant <ParserResultT <Ps, I> ...>;
    using E = make_variant <ParserResultE <Ps, I> ...>;
    return [... ps = std::move (ps)] (I & input) constexpr noexcept -> result::result <T, E>
    {
      constexpr auto impl = overload
      {
        [] (auto &&, I &, const I &) constexpr noexcept -> result::result <T, E>
        {
          return {};
        },
        [] <typename P, typename ... Rest> (auto && self, I & i, const I & backup, P && p, Rest && ... rest) constexpr noexcept -> result::result <T, E>
        {
          if (auto res = std::forward <P> (p) (i); is_success (res))
          {
            return result::success <T> {get_success (std::move (res))};
          }
          else if (is_failure (res))
          {
            return result::failure <E> {get_failure (std::move (res))};
          }
          else
          {
            i = backup;
            return self (self, i, backup, std::forward <Rest> (rest) ...);
          }
        },
      };
      return impl (impl, input, I {input}, std::move (ps) ...);
    };
  };


  // negative_lookaheadを2回適用しても実装できるけど…頭悪い？
  // lookahead: (I -> Result <unknown, unknown>) -> I -> Result <std::tuple <>, never>
  template <std::copyable I>
  inline constexpr auto lookahead = [] <Parser <I> P> (P p) constexpr noexcept
  {
    return [p = std::move (p)] (I & input) constexpr noexcept -> result::result <std::tuple <>, never>
    {
      auto dummy = input;
      if (auto res = std::move (p) (dummy); is_success (res))
      {
        return result::success {std::tuple <> {}};
      }
      else
      {
        return {};
      }
    };
  };


  // negative_lookahead: (I -> Result <unknown, unknown>) -> I -> Result <std::tuple <>, never>
  template <std::copyable I>
  inline constexpr auto negative_lookahead = [] <Parser <I> P> (P p) constexpr noexcept
  {
    return [p = std::move (p)] (I & input) constexpr noexcept -> result::result <std::tuple <>, never>
    {
      auto dummy = input;
      if (auto res = std::move (p) (dummy); is_success (res))
      {
        return {};
      }
      else
      {
        return result::success {std::tuple <> {}};
      }
    };
  };


  // optional: (I -> Result <T, E>) -> I -> Result <std::optional <T>, E>
  template <std::copyable I>
  inline constexpr auto optional = [] <Parser <I> P> (P p) constexpr noexcept
  {
    using T = ParserResultT <P, I>;
    return or_ <I> (
      map <I> (
        std::move (p),
        [] <typename U> (U && value) constexpr noexcept { return std::optional <T> {std::forward <U> (value)}; }
      ),
      [] (I &) constexpr noexcept { return result::success {std::optional <T> {}}; }
    );
  };


  // repeat: (I -> Result <T, E>) -> I -> Result <std::vector <T>, E>
  template <typename I>
  inline constexpr auto repeat = [] <Parser <I> P> (P p, std::size_t min = 0, std::size_t max = -1zu) constexpr noexcept
  {
    return [p = std::move (p), min, max] (I & input) constexpr -> result::result <std::vector <ParserResultT <P, I>>, ParserResultE <P, I>>
    {
      std::vector <ParserResultT <P, I>> v;
      if (max < -1zu)
      {
        v.reserve (max);
      }
      std::size_t i = 0;
      for (; i < max; ++ i)
      {
        if (auto res = p (input); is_success (res))
        {
          v.push_back (get_success (std::move (res)));
        }
        else if (is_failure (res))
        {
          return as_failure (std::move (res));
        }
        else
        {
          break;
        }
      }
      if (i < min)
      {
        return {};
      }
      else
      {
        return result::success {std::move (v)};
      }
    };
  };


  // separated: ((I -> Result <T, E>), (I -> Result <unknown, E2>)) -> I -> Result <std::vector <T>, E | E2>
  template <typename I>
  inline constexpr auto separated = [] <Parser <I> P, Parser <I> S> (P p, S s) constexpr noexcept
  {
    using E = make_variant <ParserResultE <P, I>, ParserResultE <S, I>>;
    return [p = std::move (p), s = std::move (s)] (I & input) constexpr -> result::result <std::vector <ParserResultT <P, I>>, E>
    {
      std::vector <ParserResultT <P, I>> v;
      while (true)
      {
        if (auto res = p (input); is_success (res))
        {
          v.push_back (get_success (std::move (res)));
        }
        else if (is_failure (res))
        {
          return result::failure <E> {get_failure (std::move (res))};
        }
        else
        {
          break;
        }
        if (auto res = s (input); is_success (res))
        {
          continue;
        }
        else if (is_failure (res))
        {
          return result::failure <E> {get_failure (std::move (res))};
        }
        else
        {
          break;
        }
      }
      return result::success {std::move (v)};
    };
  };

  #if defined (__GNUC__)
    #pragma GCC diagnostic pop
  #endif
}

#define USING_CHINO_PARSER_COMBINATORS(I) \
  inline constexpr auto map                = chino::parser::map <I>; \
  inline constexpr auto flat_map           = chino::parser::flat_map <I>; \
  inline constexpr auto catch_error        = chino::parser::catch_error <I>; \
  inline constexpr auto and_               = chino::parser::and_ <I>; \
  inline constexpr auto and_with_skip      = chino::parser::and_with_skip <I>; \
  inline constexpr auto or_                = chino::parser::or_ <I>; \
  inline constexpr auto lookahead          = chino::parser::lookahead <I>; \
  inline constexpr auto negative_lookahead = chino::parser::negative_lookahead <I>; \
  inline constexpr auto optional           = chino::parser::optional <I>; \
  inline constexpr auto repeat             = chino::parser::repeat <I>; \
  inline constexpr auto separated          = chino::parser::separated <I>

#endif
