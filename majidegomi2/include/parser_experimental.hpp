#ifndef CHINO_PARSER_HPP
#define CHINO_PARSER_HPP
#include <chino/parser/result.hpp>
#include <tuple>
#include <vector>

namespace chino::parser
{
  template <typename P, typename I>
  using success_type = result::success_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  using failure_type = result::failure_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  concept Parser = result::Result <std::invoke_result_t <P, I &>>;

  namespace detail
  {
    template <typename T>
    concept is_empty_and_trivially_copy_constuructible = std::is_empty_v <T> && std::is_trivially_copy_constructible_v <T>;

    template <typename T>
    using remove_cvref_if_empty = std::conditional_t <is_empty_and_trivially_copy_constuructible <std::remove_cvref_t <T>>, std::remove_cvref_t <T>, T>;
  }

  // --------------------------------
  //  parser combinators
  // --------------------------------
  #if defined (__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpadded"
  #endif

  // NOTE: 高階関数の返すlambda式で、キャプチャしたparser(変数)をmoveしてはならない
  //       そのparser(変数)がくり返し使われる可能性があるため

  /* template <typename T> */
  /* concept is_not_void = not std::same_as <T, void>; */

  // map: (I -> Result <T, E>, T -> U) -> I -> Result <U, E>
  template <typename I>
  inline constexpr auto map = [] <Parser <I> P, typename F> (P && p, F && f) constexpr noexcept
  {
    static_assert (requires {typename std::invoke_result_t <F, success_type <P, I>>;});
    return [captured = std::tuple <detail::remove_cvref_if_empty <P>, detail::remove_cvref_if_empty <F>> {std::forward <P> (p), std::forward <F> (f)}] (I & input) constexpr noexcept
    {
      return result::map (std::get <0> (captured) (input), std::get <1> (captured));
    };
  };


  // flat_map: (I -> Result <T, E>, T -> Result <U, E2>) -> I -> Result <U, E | E2>
  template <typename I>
  inline constexpr auto flat_map = [] <Parser <I> P, typename F> (P && p, F && f) constexpr noexcept
  {
    static_assert (result::Result <std::invoke_result_t <F, success_type <P, I>, I &>>);
    return [captured = std::tuple <detail::remove_cvref_if_empty <P>, detail::remove_cvref_if_empty <F>> {std::forward <P> (p), std::forward <F> (f)}] (I & input) constexpr noexcept
    {
      return result::and_then (std::get <0> (captured) (input), [&] <typename R> (R && res) constexpr noexcept { return std::get <1> (captured) (std::forward <R> (res), input); });
    };
  };


  // failureのflat_map
  // catch_error: (I -> Result <T, E>, E -> Result <U, E2>) -> I -> Result <T | U, E2>
  template <typename I>
  inline constexpr auto catch_error = [] <Parser <I> P, typename F> (P && p, F && f) constexpr noexcept
  {
    static_assert (result::Result <std::invoke_result_t <F, success_type <P, I>, I &>>);
    return [captured = std::tuple <detail::remove_cvref_if_empty <P>, detail::remove_cvref_if_empty <F>> {std::forward <P> (p), std::forward <F> (f)}] (I & input) constexpr noexcept
    {
      return result::catch_error (std::get <0> (captured) (input), [&] <typename R> (R && res) constexpr noexcept { return std::get <1> (captured) (std::forward <R> (res), input); });
    };
  };


  // and_: ((I -> Result <Ts, Es>) ...) -> I -> Result <std::tuple <Ts ...>, make_variant <Es ...>>
  template <typename I>
  inline constexpr auto and_ = [] <Parser <I> ... Ps> (Ps && ... ps) constexpr noexcept
  {
    using T = std::tuple <success_type <Ps, I> ...>;
    using E = make_variant <failure_type <Ps, I> ...>;
    return [captured = std::tuple <Ps ...> {std::forward <Ps> (ps) ...}] (I & input) constexpr noexcept -> result::result <T, E>
    {
      constexpr auto impl = [] <std::size_t i, typename U> (auto & self, I & in, U && value, auto & captured_) constexpr noexcept -> result::result <T, E>
      {
        if constexpr (i < sizeof ... (Ps))
        {
          if (auto res = std::get <i> (captured_) (in); is_success (res))
          {
            return self.template operator () <i + 1> (self, in, std::tuple_cat (std::forward <U> (value), std::make_tuple (get_success (std::move (res)))), captured_);
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
        else
        {
          return result::success <T> {std::forward <U> (value)};
        }
      };
      return impl.template operator () <0> (impl, input, std::tuple <> {}, captured);
    };
  };


  // or_: ((I -> Result <Ts, Es>) ...) -> I -> Result <make_variant <Ts ...>, make_variant <Es ...>>
  template <std::copyable I>
  inline constexpr auto or_ = [] <Parser <I> ... Ps> (Ps && ... ps) constexpr noexcept
  {
    using T = make_variant <success_type <Ps, I> ...>;
    using E = make_variant <failure_type <Ps, I> ...>;
    return [captured = std::tuple <Ps ...> {std::forward <Ps> (ps) ...}] (I & input) constexpr noexcept -> result::result <T, E>
    {
      constexpr auto impl = [] <std::size_t i> (auto && self, I & in, const I & backup, auto & captured_) constexpr noexcept -> result::result <T, E>
      {
        if constexpr (i < sizeof ... (Ps))
        {
          if (auto res = std::get <i> (captured_) (in); is_success (res))
          {
            return result::success <T> {get_success (std::move (res))};
          }
          else if (is_failure (res))
          {
            return result::failure <E> {get_failure (std::move (res))};
          }
          else
          {
            in = backup;
            return self.template operator () <i + 1> (self, in, backup, captured_);
          }
        }
        else
        {
          return {};
        }
      };
      return impl.template operator () <0> (impl, input, I {input}, captured);
    };
  };


  // inputを動かさずに結果を得る
  // lookahead: (I -> Result <T, E>) -> I -> Result <T, E>
  template <std::copyable I>
  inline constexpr auto lookahead = [] <Parser <I> P> (P && p) constexpr noexcept
  {
    using T = success_type <P, I>;
    using E = failure_type <P, I>;
    return [captured = std::tuple <P> {std::forward (p)}] (I & input) constexpr noexcept -> result::result <T, E>
    {
      auto dummy = input;
      return std::get <0> (captured) (dummy);
    };
  };


  // 例外を握りつぶす
  // negative_lookahead: (I -> Result <unknown, unknown>) -> I -> Result <std::tuple <>, never>
  template <std::copyable I>
  inline constexpr auto negative_lookahead = [] <Parser <I> P> (P && p) constexpr noexcept
  {
    return [captured = std::tuple <P> {std::forward <P> (p)}] (I & input) constexpr noexcept -> result::result <std::tuple <>, never>
    {
      auto dummy = input;
      if (auto res = std::get <0> (captured) (dummy); is_success (res))
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
  inline constexpr auto optional = [] <Parser <I> P> (P && p) constexpr noexcept
  {
    using T = success_type <P, I>;
    return or_ <I> (
      map <I> (
        std::forward <P> (p),
        [] <typename U> (U && value) constexpr noexcept { return std::optional <T> {std::forward <U> (value)}; }
      ),
      [] (I &) constexpr noexcept { return result::success {std::optional <T> {}}; }
    );
  };


  // repeat: (I -> Result <T, E>) -> I -> Result <std::vector <T>, E>
  template <typename I>
  inline constexpr auto repeat = [] <Parser <I> P> (P && p, std::size_t min = 0, std::size_t max = -1zu) constexpr noexcept
  {
    return [captured = std::tuple <P> {std::forward <P> (p)}, min, max] (I & input) constexpr -> result::result <std::vector <success_type <P, I>>, failure_type <P, I>>
    {
      std::vector <success_type <P, I>> v;
      if (max < -1zu)
      {
        v.reserve (max);
      }
      std::size_t i = 0;
      for (; i < max; ++ i)
      {
        if (auto res = std::get <0> (captured) (input); is_success (res))
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
  inline constexpr auto separated = [] <Parser <I> P, Parser <I> S> (P && p, S && s) constexpr noexcept
  {
    using E = make_variant <failure_type <P, I>, failure_type <S, I>>;
    return [captured = std::tuple <P, S> {std::forward <P> (p), std::forward <S> (s)}] (I & input) constexpr -> result::result <std::vector <success_type <P, I>>, E>
    {
      std::vector <success_type <P, I>> v;
      while (true)
      {
        if (auto res = std::get <0> (captured) (input); is_success (res))
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
        if (auto res = std::get <1> (captured) (input); is_success (res))
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
  inline constexpr auto or_                = chino::parser::or_ <I>; \
  inline constexpr auto lookahead          = chino::parser::lookahead <I>; \
  inline constexpr auto negative_lookahead = chino::parser::negative_lookahead <I>; \
  inline constexpr auto optional           = chino::parser::optional <I>; \
  inline constexpr auto repeat             = chino::parser::repeat <I>; \
  inline constexpr auto separated          = chino::parser::separated <I>

#endif
