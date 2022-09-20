#ifndef CHINO_PARSER_HPP
#define CHINO_PARSER_HPP
#include <chino/parser/result.hpp>
#include <tuple>
#include <optional>
#include <vector>

namespace chino::parser
{
  template <typename P, typename I>
  using undefined_type = result::undefined_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  using success_type = result::success_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  using failure_type = result::failure_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  concept Parser = result::Result <std::invoke_result_t <P, I &>>;

  // --------------------------------
  //  parser combinators
  // --------------------------------
  // NOTE: 高階関数の返すlambda式で、キャプチャしたparser(変数)をmoveしてはならない
  //       そのparser(変数)がくり返し使われる可能性があるため

  // NOTE: 空クラス かつ is_trivially_copy_constructible_v な関数オブジェクトはコピーしたほうが(メモリ)効率がいい
  namespace detail
  {
    template <typename T>
    concept is_empty_and_trivially_copy_constructible = std::is_empty_v <T> && std::is_trivially_copy_constructible_v <T>;

    template <typename T>
    using remove_cvref_if_empty = std::conditional_t <is_empty_and_trivially_copy_constructible <std::remove_cvref_t <T>>, std::remove_cvref_t <T>, T>;

    template <typename ... Ts>
    inline constexpr auto make_capture (Ts && ... xs) noexcept
    {
      return std::tuple <remove_cvref_if_empty <Ts> ...> {std::forward <Ts> (xs) ...};
    }

    template <typename ... Ts>
    using last_t = typename decltype ((std::type_identity <never> {}, ..., std::type_identity <Ts> {}))::type;
  }

  /* template <typename T> */
  /* concept is_not_void = not std::same_as <T, void>; */

  // map: (I -> Result <T, E>, T -> U) -> I -> Result <U, E>
  inline constexpr auto map = [] <typename P, typename F> (P && p, F && f) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p), std::forward <F> (f))] <typename I> (I & input) constexpr noexcept
    {
      static_assert (requires {typename std::invoke_result_t <F, success_type <P, I>>;});
      return result::map (std::get <0> (captured) (input), std::get <1> (captured));
    };
  };


  // flat_map: (I -> Result <T, E>, T -> Result <U, E2>) -> I -> Result <U, E | E2>
  inline constexpr auto flat_map = [] <typename P, typename F> (P && p, F && f) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p), std::forward <F> (f))] <typename I> (I & input) constexpr noexcept
    {
      static_assert (result::Result <std::invoke_result_t <F, success_type <P, I>, I &>>);
      return result::and_then (std::get <0> (captured) (input), [&] <typename R> (R && res) constexpr noexcept { return std::get <1> (captured) (std::forward <R> (res), input); });
    };
  };


  // failureのflat_map
  // catch_error: (I -> Result <T, E>, E -> Result <U, E2>) -> I -> Result <T | U, E2>
  inline constexpr auto catch_error = [] <typename P, typename F> (P && p, F && f) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p), std::forward <F> (f))] <typename I> (I & input) constexpr noexcept
    {
      static_assert (result::Result <std::invoke_result_t <F, success_type <P, I>, I &>>);
      return result::catch_error (std::get <0> (captured) (input), [&] <typename R> (R && res) constexpr noexcept { return std::get <1> (captured) (std::forward <R> (res), input); });
    };
  };


  // and_: ((I -> Result <Ts, Es>) ...) -> I -> Result <std::tuple <Ts ...>, make_variant <Es ...>>
  inline constexpr auto and_ = [] <typename ... Ps> (Ps && ... ps) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <Ps> (ps) ...)] <typename I> requires (Parser <Ps, I> && ...) (I & input) constexpr noexcept
    {
      using Undefined = make_variant <undefined_type <Ps, I> ...>;
      using T = std::tuple <success_type <Ps, I> ...>;
      using E = make_variant <failure_type <Ps, I> ...>;
      constexpr auto impl = [] <std::size_t i, typename U> (auto & self, I & in, U && value, auto & captured_) constexpr noexcept -> result::make_result3 <Undefined, T, E>
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
            return as_undefined (res);
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
  inline constexpr auto or_ = [] <typename ... Ps> (Ps && ... ps) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <Ps> (ps) ...)] <std::copyable I> requires (Parser <Ps, I> && ...) (I & input) constexpr noexcept
    {
      using Undefined = detail::last_t <result::undefined, undefined_type <Ps, I> ...>;
      using T = make_variant <success_type <Ps, I> ...>;
      using E = make_variant <failure_type <Ps, I> ...>;
      constexpr auto impl = [] <std::size_t i> (auto && self, I & in, const I & backup, auto & captured_) constexpr noexcept -> result::make_result3 <Undefined, T, E>
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
        else if constexpr (not std::same_as <Undefined, never>)
        {
          return {};
        }
        else
        {
          unreachable ();
        }
      };
      return impl.template operator () <0> (impl, input, I {input}, captured);
    };
  };


  // inputを動かさずに結果を得る
  // lookahead: (I -> Result <T, E>) -> I -> Result <T, E>
  inline constexpr auto lookahead = [] <typename P> (P && p) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p))] <std::copyable I> requires Parser <P, I> (I & input) constexpr noexcept
    {
      auto dummy = input;
      return std::get <0> (captured) (dummy);
    };
  };


  // 例外を握りつぶす
  // negative_lookahead: (I -> Result <unknown, unknown>) -> I -> Result <std::tuple <>, never>
  inline constexpr auto negative_lookahead = [] <typename P> (P && p) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p))] <std::copyable I> requires Parser <P, I> (I & input) constexpr noexcept -> result::optional <std::tuple <>>
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
  inline constexpr auto optional = [] <typename P> (P && p) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p))] <std::copyable I> (I & input) constexpr noexcept -> result::make_result2 <std::optional <success_type <P, I>>, failure_type <P, I>>
    {
      if (auto res = std::get <0> (captured) (input); is_success (res))
      {
        return result::success {std::optional {get_success (res)}};
      }
      else if (is_failure (res))
      {
        return as_failure (res);
      }
      else
      {
        return result::success {std::optional <success_type <P, I>> {}};
      }
    };
  };


  // repeat: (I -> Result <T, E>) -> I -> Result <std::vector <T>, E>
  template <std::size_t min = 0, std::size_t max = -1zu, bool reserve = false>
  inline constexpr auto repeat = [] <typename P> (P && p) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p))] <typename I> (I & input) constexpr -> result::make_result3 <std::conditional_t <min == 0, never, result::undefined>, std::vector <success_type <P, I>>, failure_type <P, I>>
    {
      auto & [p_] = captured;
      std::vector <success_type <P, I>> v;
      if constexpr (reserve)
      {
        v.reserve (max);
      }
      std::size_t i = 0;
      for (; i < max; ++ i)
      {
        if (auto res = p_ (input); is_success (res))
        {
          v.push_back (get_success (std::move (res)));
        }
        else if (is_failure (res))
        {
          return as_failure (std::move (res));
        }
        else
        {
          static_cast <void> (as_undefined (res));
          break;
        }
      }
      if constexpr (min > 0)
      {
        if (i < min)
        {
          return {};
        }
      }
      return result::success {std::move (v)};
    };
  };


  // separated: ((I -> Result <T, E>), (I -> Result <unknown, E2>)) -> I -> Result <std::vector <T>, E | E2>
  inline constexpr auto separated = [] <typename P, typename S> (P && p, S && s) constexpr noexcept
  {
    return [captured = detail::make_capture (std::forward <P> (p), std::forward <S> (s))] <typename I, typename E = make_variant <failure_type <P, I>, failure_type <S, I>>> (I & input) constexpr -> result::make_result2 <std::vector <success_type <P, I>>, E>
    {
      auto & [p_, s_] = captured;
      std::vector <success_type <P, I>> v;
      while (true)
      {
        if (auto res = p_ (input); is_success (res))
        {
          v.push_back (get_success (std::move (res)));
        }
        else if (is_failure (res))
        {
          return result::failure <E> {get_failure (std::move (res))};
        }
        else
        {
          static_cast <void> (as_undefined (res));
          break;
        }
        if (auto res = s_ (input); is_success (res))
        {
          continue;
        }
        else if (is_failure (res))
        {
          return result::failure <E> {get_failure (std::move (res))};
        }
        else
        {
          static_cast <void> (as_undefined (res));
          break;
        }
      }
      return result::success {std::move (v)};
    };
  };
}

#endif
