#ifndef CHINO_PARSER_HPP
#define CHINO_PARSER_HPP
#include <chino/make_variant.hpp>
#include <chino/type_traits/copy_cvref_from.hpp>
#include <tuple>
#include <optional>
#include <variant>
#include <vector>
#include <stdexcept>

namespace chino::parser
{
  /* struct never */
  /* { */
  /*   constexpr never () noexcept = delete; */

  /*   template <typename T> */
  /*   constexpr operator const T & () const & */
  /*   { */
  /*     throw std::runtime_error {"bad never access"}; */
  /*   } */

  /*   template <typename T> */
  /*   constexpr operator const T && () const && */
  /*   { */
  /*     throw std::runtime_error {"bad never access"}; */
  /*   } */

  /*   template <typename T> */
  /*   constexpr operator T & () & */
  /*   { */
  /*     throw std::runtime_error {"bad never access"}; */
  /*   } */

  /*   template <typename T> */
  /*   constexpr operator T && () && */
  /*   { */
  /*     throw std::runtime_error {"bad never access"}; */
  /*   } */
  /* }; */

  /* [[noreturn]] inline auto unreachable () noexcept -> never */
  /* { */
  /*   std_unreachable (); */
  /* } */

  namespace result
  {
    template <typename T>
    struct success
    {
      T value;
    };

    template <typename T>
    success (T &&) -> success <std::remove_cvref_t <T>>;

    template <typename E>
    struct failure
    {
      E error;
    };

    template <typename E>
    failure (E &&) -> failure <std::remove_cvref_t <E>>;

    template <typename T, typename E>
    using result = std::variant <std::monostate, success <T>, failure <E>>;

    template <typename>
    struct result_traits;

    template <typename T, typename E>
    struct result_traits <result <T, E>>
    {
      using success_type = T;
      using failure_type = E;

      template <typename R>
      static constexpr auto is_success (const R & r) noexcept
      {
        return std::holds_alternative <success <T>> (r);
      }
      template <typename R>
      static constexpr auto is_failure (const R & r) noexcept
      {
        return std::holds_alternative <failure <E>> (r);
      }
      template <typename R>
      static constexpr auto as_success (R && r) noexcept -> decltype (auto)
      {
        return std::get <success <T>> (std::forward <R> (r));
      }
      template <typename R>
      static constexpr auto as_failure (R && r) noexcept -> decltype (auto)
      {
        return std::get <failure <E>> (std::forward <R> (r));
      }
    };

    template <typename T>
    struct result_traits <success <T>>
    {
      using success_type = T;
      using failure_type = never;

      template <typename R>
      static constexpr auto is_success (const R &) noexcept
      {
        return true;
      }
      template <typename R>
      static constexpr auto is_failure (const R &) noexcept
      {
        return false;
      }
      template <typename R>
      static constexpr auto as_success (R && r) noexcept -> decltype (auto)
      {
        return std::forward <R> (r);
      }
      template <typename R>
      static constexpr auto as_failure (R &&) noexcept -> copy_cvref_from <failure <never>, R &&>
      {
        return unreachable ();
      }
    };

    template <typename E>
    struct result_traits <failure <E>>
    {
      using success_type = never;
      using failure_type = E;

      template <typename R>
      static constexpr auto is_success (const R &) noexcept
      {
        return false;
      }
      template <typename R>
      static constexpr auto is_failure (const R &) noexcept
      {
        return true;
      }
      template <typename R>
      static constexpr auto as_success (R &&) noexcept -> copy_cvref_from <success <never>, R &&>
      {
        return unreachable ();
      }
      template <typename R>
      static constexpr auto as_failure (R && r) noexcept -> decltype (auto)
      {
        return std::forward <R> (r);
      }
    };

    template <typename T>
    struct result_traits <std::optional <success <T>>>
    {
      using success_type = T;
      using failure_type = never;

      template <typename R>
      static constexpr auto is_success (const R & r) noexcept
      {
        return static_cast <bool> (r);
      }
      template <typename R>
      static constexpr auto is_failure (const R &) noexcept
      {
        return false;
      }
      template <typename R>
      static constexpr auto as_success (R && r) noexcept -> decltype (auto)
      {
        return * std::forward <R> (r);
      }
      template <typename R>
      static constexpr auto as_failure (R &&) noexcept -> copy_cvref_from <failure <never>, R &&>
      {
        return unreachable ();
      }
    };

    template <typename E>
    struct result_traits <std::optional <failure <E>>>
    {
      using success_type = never;
      using failure_type = E;

      template <typename R>
      static constexpr auto is_success (const R &) noexcept
      {
        return false;
      }
      template <typename R>
      static constexpr auto is_failure (const R & r) noexcept
      {
        return static_cast <bool> (r);
      }
      template <typename R>
      static constexpr auto as_success (R &&) noexcept -> copy_cvref_from <success <never>, R &&>
      {
        return unreachable ();
      }
      template <typename R>
      static constexpr auto as_failure (R && r) noexcept -> decltype (auto)
      {
        return * std::forward <R> (r);
      }
    };
    static_assert (std::is_same_v <decltype (result_traits <std::optional <failure <int>>>::as_failure (std::declval <std::optional <failure <int>> &> ())), failure <int> &>);
    static_assert (std::is_same_v <decltype (result_traits <std::optional <failure <int>>>::as_failure (std::declval <std::optional <failure <int>> &&> ())), failure <int> &&>);
    static_assert (std::is_same_v <decltype (result_traits <std::optional <failure <int>>>::as_failure (std::declval <const std::optional <failure <int>> &> ())), const failure <int> &>);
    static_assert (std::is_same_v <decltype (result_traits <std::optional <failure <int>>>::as_failure (std::declval <const std::optional <failure <int>>> ())), const failure <int> &&>);

    template <typename T>
    concept Result = requires
    {
      typename result_traits <std::remove_cvref_t <T>>::success_type;
      typename result_traits <std::remove_cvref_t <T>>::failure_type;
    };

    template <Result R>
    using success_type = typename result_traits <std::remove_cvref_t <R>>::success_type;

    template <Result R>
    using failure_type = typename result_traits <std::remove_cvref_t <R>>::failure_type;

    template <Result R>
    inline constexpr auto is_success (const R & r) noexcept
    {
      return result_traits <std::remove_cvref_t <R>>::is_success (r);
    }

    template <Result R>
    inline constexpr auto is_failure (const R & r) noexcept
    {
      return result_traits <std::remove_cvref_t <R>>::is_failure (r);
    }

    template <Result R>
    inline constexpr auto as_success (R && r) noexcept -> decltype (auto)
    {
      return result_traits <std::remove_cvref_t <R>>::as_success (std::forward <R> (r));
    }

    template <Result R>
    inline constexpr auto as_failure (R && r) noexcept -> decltype (auto)
    {
      return result_traits <std::remove_cvref_t <R>>::as_failure (std::forward <R> (r));
    }

    template <Result R>
    inline constexpr auto get_success (R && r) noexcept -> decltype (auto)
    {
      return (as_success (std::forward <R> (r)).value);
    }

    template <Result R>
    inline constexpr auto get_failure (R && r) noexcept -> decltype (auto)
    {
      return (as_failure (std::forward <R> (r)).error);
    }

    template <Result R, typename F, typename G, typename H>
    inline constexpr auto match (R && r, F && f, G && g, H && h) noexcept
    {
      using ResultF = decltype (f (get_success (std::forward <R> (r))));
      using ResultG = decltype (g (get_failure (std::forward <R> (r))));
      using ResultH = decltype (h ());
      static_assert (std::is_same_v <ResultF, ResultG>);
      static_assert (std::is_same_v <ResultF, ResultH>);
      if (is_success (r))
      {
        return f (get_success (std::forward <R> (r)));
      }
      else if (is_failure (r))
      {
        return g (get_failure (std::forward <R> (r)));
      }
      else
      {
        return h ();
      }
    }

    template <Result R, typename F>
    inline constexpr auto map (R && r, F && f) noexcept (noexcept (f (get_success (std::forward <R> (r)))))
    -> result <std::remove_cvref_t <decltype (f (get_success (std::forward <R> (r))))>, failure_type <R>>
    {
      if (is_success (r))
      {
        return success {f (get_success (std::forward <R> (r)))};
      }
      else if (is_failure (r))
      {
        return as_failure (std::forward <R> (r));
      }
      else
      {
        return {};
      }
    }

    template <Result R, typename F>
    inline constexpr auto and_then (R && r, F && f) noexcept (noexcept (f (get_success (std::forward <R> (r)))))
    -> result <success_type <decltype (f (get_success (std::forward <R> (r))))>, make_variant <failure_type <R>, failure_type <decltype (f (get_success (std::forward <R> (r))))>>>
    {
      if (is_success (r))
      {
        return f (get_success (std::forward <R> (r)));
      }
      else if (is_failure (r))
      {
        return failure <make_variant <failure_type <R>, failure_type <decltype (f (get_success (std::forward <R> (r))))>>> {get_failure (std::forward <R> (r))};
      }
      else
      {
        return {};
      }
    }

    template <Result R, typename F>
    inline constexpr auto catch_error (R && r, F && f) noexcept (noexcept (f (get_failure (std::forward <R> (r)))))
    -> result <make_variant <success_type <R>, success_type <decltype (f (get_failure (std::forward <R> (r))))>>, failure_type <decltype (f (get_failure (std::forward <R> (r))))>>
    {
      if (is_success (r))
      {
        return success <make_variant <success_type <R>, success_type <decltype (f (get_failure (std::forward <R> (r))))>>> {get_success (std::forward <R> (r))};
      }
      else if (is_failure (r))
      {
        return f (get_failure (std::forward <R> (r)));
      }
      else
      {
        return {};
      }
    }
  }

  template <typename P, typename I>
  using ParserResultT = result::success_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  using ParserResultE = result::failure_type <std::invoke_result_t <P, I &>>;


  // --------------------------------
  //  parser combinators
  // --------------------------------

  // map: (Parser <T>, T -> U) -> Parser <U>
  inline constexpr auto map = [] <typename P, typename F> (P p, F f) constexpr noexcept
  {
    return [p = std::move (p), f = std::move (f)] <typename I> requires requires (I & input)
    {
      {result::map (std::move (p) (input), std::move (f))};
    }
    (I & input) constexpr noexcept
    {
      return result::map (std::move (p) (input), std::move (f));
    };
  };


  // flat_map: (Parser <T>, T -> r <U>) -> Parser <U>
  inline constexpr auto flat_map = [] <typename P, typename F> (P p, F f) constexpr noexcept
  {
    return [p = std::move (p), f = std::move (f)] <typename I> requires requires (I & input)
    {
      {result::and_then (std::move (p) (input), std::move (f))} -> result::Result;
    }
    (I & input) constexpr noexcept
    {
      return result::and_then (std::move (p) (input), std::move (f));
    };
  };


  // failureのflat_map
  // recover: (Parser <T>, std::string -> r <U>) -> Parser <U>
  inline constexpr auto recover = [] <typename P, typename F> (P p, F f) constexpr noexcept
  {
    return [p = std::move (p), f = std::move (f)] <typename I> requires requires (I & input)
    {
      {result::catch_error (std::move (p) (input), std::move (f))} -> result::Result;
    }
    (I & input) constexpr noexcept
    {
      return result::catch_error (std::move (p) (input), std::move (f));
    };
  };


  // raise: (() -> T) -> Parser <never, T>
  /* inline constexpr auto raise = [] <typename F> requires requires (F f) */
  /* { */
  /*   {std::move (f) ()}; */
  /* } (F f) constexpr noexcept */
  /* { */
  /*   return [f = std::move (f)] <typename I> (I &) constexpr noexcept -> result::result <never, std::remove_cvref_t <decltype (std::move (f) ())>> */
  /*   { */
  /*     return result::failure {std::move (f) ()}; */
  /*   }; */
  /* }; */


  // and_: (Parser <Ts> ...) -> Parser <std::tuple <ParserResultT <Ts> ...>>
  namespace impl
  {
    template <typename E, typename I, typename ... Ts, typename P, typename ... Ps>
    inline constexpr auto and_ (I & input, std::tuple <Ts ...> && t, P && p, Ps && ... ps) noexcept -> result::result <std::tuple <Ts ..., ParserResultT <P, I>, ParserResultT <Ps, I> ...>, E>
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
          return and_ <E> (input, std::tuple_cat (std::move (t), std::make_tuple (get_success (std::move (res)))), std::forward <Ps> (ps) ...);
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
  }
  inline constexpr auto and_ = [] <typename ... Ps> (Ps ... ps) constexpr noexcept
  {
    return [... ps = std::move (ps)] <typename I> (I & input) constexpr noexcept -> result::result <std::tuple <ParserResultT <Ps, I> ...>, make_variant <ParserResultE <Ps, I> ...>>
    {
      return impl::and_ <make_variant <ParserResultE <Ps, I> ...>> (input, std::tuple <> {}, std::move (ps) ...);
    };
  };


  // or_: (Parser <Ts> ...) -> Parser <make_variant <std::variant <ParserResultT <Ts> ...>>>
  namespace impl
  {
    template <typename T, typename E, typename I, typename P, typename ... Ps>
    inline constexpr auto or_ (I & input, P && p, Ps && ... ps) noexcept -> result::result <T, E>
    {
      auto backup = input;
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
          input = backup;
          return or_ <T, E> (input, std::forward <Ps> (ps) ...);
        }
      }
    }
  }
  inline constexpr auto or_ = [] <typename ... Ps> (Ps ... ps) constexpr noexcept
  {
    return [... ps = std::move (ps)] <typename I> (I & input) constexpr noexcept -> result::result <make_variant <ParserResultT <Ps, I> ...>, make_variant <ParserResultE <Ps, I> ...>>
    {
      return impl::or_ <make_variant <ParserResultT <Ps, I> ...>, make_variant <ParserResultE <Ps, I> ...>> (input, std::move (ps) ...);
    };
  };


  // optional: (Parser <T>) -> Parser <std::optional <T>>
  inline constexpr auto optional = [] <typename P> (P p) constexpr noexcept
  {
    return [p = std::move (p)] <typename I> (I & input) constexpr noexcept -> result::result <std::optional <ParserResultT <P, I>>, ParserResultE <P, I>>
    {
      auto res = std::move (p) (input);
      if (is_success (res))
      {
        return result::success <std::optional <ParserResultT <P, I>>> {get_success (std::move (res))};
      }
      else if (is_failure (res))
      {
        return as_failure (std::move (res));
      }
      else
      {
        return result::success <std::optional <ParserResultT <P, I>>> {};
      }
    };
  };


  // repeat: (Parser <T>) -> Parser <std::vector <T>>
  inline constexpr auto repeat = [] <typename P> (P p) constexpr noexcept
  {
    return [p = std::move (p)] <typename I> (I & input) constexpr -> result::result <std::vector <ParserResultT <P, I>>, ParserResultE <P, I>>
    {
      std::vector <ParserResultT <P, I>> v;
      while (true)
      {
        auto res = p (input);
        if (is_success (res))
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
      return result::success {std::move (v)};
    };
  };


  // disallow_empty: (Parser <T>) -> Parser <T>
  inline constexpr auto disallow_empty = [] <typename P> (P p) constexpr noexcept
  {
    return flat_map (std::move (p), [] <typename T> (T && v) constexpr noexcept -> result::result <T, never>
    {
      if (v.empty ())
      {
        return {};
      }
      else
      {
        return result::success {std::forward <T> (v)};
      }
    });
  };


  // more: (Parser <T>) -> Parser <std::vector <T>>
  inline constexpr auto more = [] <typename P> (P p) constexpr noexcept
  {
    return disallow_empty (repeat (std::move (p)));
  };

}

#endif
