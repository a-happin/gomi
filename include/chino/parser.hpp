#ifndef CHINO_PARSER_HPP
#define CHINO_PARSER_HPP
#include <tuple>
#include <optional>
#include <variant>
#include <vector>
#include <stdexcept>

namespace chino::parser
{
  struct never
  {
    constexpr never () noexcept = delete;

    template <typename T>
    constexpr operator const T & () const &
    {
      throw std::runtime_error {"bad never access"};
    }

    template <typename T>
    constexpr operator const T && () const &&
    {
      throw std::runtime_error {"bad never access"};
    }

    template <typename T>
    constexpr operator T & () &
    {
      throw std::runtime_error {"bad never access"};
    }

    template <typename T>
    constexpr operator T && () &&
    {
      throw std::runtime_error {"bad never access"};
    }
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

    template <Result R, typename F, typename G, typename H>
    inline constexpr auto match (R && result, F && f, G && g, H && h) noexcept
    {
      using ResultF = decltype (f (get_success (std::forward <R> (result))));
      using ResultG = decltype (g (get_failure (std::forward <R> (result))));
      using ResultH = decltype (h ());
      static_assert (std::is_same_v <ResultF, ResultG>);
      static_assert (std::is_same_v <ResultF, ResultH>);
      if (is_success (result))
      {
        return f (get_success (std::forward <R> (result)));
      }
      else if (is_failure (result))
      {
        return g (get_failure (std::forward <R> (result)));
      }
      else
      {
        return h ();
      }
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
    inline constexpr auto and_then (R && result, F && f) noexcept -> result_t <result_traits::success_type <decltype (f (get_success (std::forward <R> (result))))>, make_variant_t <result_traits::failure_type <R>, result_traits::failure_type <decltype (f (get_success (std::forward <R> (result))))>>>
    {
      if (is_success (result))
      {
        return f (get_success (std::forward <R> (result)));
      }
      else if (is_failure (result))
      {
        return failure <make_variant_t <result_traits::failure_type <R>, result_traits::failure_type <decltype (f (get_success (std::forward <R> (result))))>>> {get_failure (std::forward <R> (result))};
      }
      else
      {
        return {};
      }
    }

    template <Result R, typename F>
    inline constexpr auto catch_error (R && result, F && f) noexcept -> result_t <make_variant_t <result_traits::success_type <R>, result_traits::success_type <decltype (f (get_failure (std::forward <R> (result))))>>, result_traits::failure_type <decltype (f (get_failure (std::forward <R> (result))))>>
    {
      if (is_success (result))
      {
        return success <make_variant_t <result_traits::success_type <R>, result_traits::success_type <decltype (f (get_failure (std::forward <R> (result))))>>> {get_success (std::forward <R> (result))};
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

  template <typename P, typename I>
  using ParserResultT = result::result_traits::success_type <std::invoke_result_t <P, I &>>;

  template <typename P, typename I>
  using ParserResultE = result::result_traits::failure_type <std::invoke_result_t <P, I &>>;


  // --------------------------------
  //  parser combinators
  // --------------------------------

  // map: (Parser <T>, T -> U) -> Parser <U>
  inline constexpr auto map = [] <typename P, typename F> (P p, F f) constexpr noexcept
  {
    return [p = std::move (p), f = std::move (f)] <typename I> requires requires (I & input)
    {
      {result::map (p (input), std::move (f))};
    }
    (I & input) constexpr noexcept
    {
      return result::map (p (input), std::move (f));
    };
  };


  // flat_map: (Parser <T>, T -> Result <U>) -> Parser <U>
  inline constexpr auto flat_map = [] <typename P, typename F> (P p, F f) constexpr noexcept
  {
    return [p = std::move (p), f = std::move (f)] <typename I> requires requires (I & input)
    {
      {result::and_then (p (input), std::move (f))} -> result::Result;
    }
    (I & input) constexpr noexcept
    {
      return result::and_then (p (input), std::move (f));
    };
  };


  // failureのflat_map
  // recover: (Parser <T>, std::string -> Result <U>) -> Parser <U>
  inline constexpr auto recover = [] <typename P, typename F> (P p, F f) constexpr noexcept
  {
    return [p = std::move (p), f = std::move (f)] <typename I> requires requires (I & input)
    {
      {result::catch_error (p (input), std::move (f))} -> result::Result;
    }
    (I & input) constexpr noexcept
    {
      return result::catch_error (p (input), std::move (f));
    };
  };


  // and_: (Parser <Ts> ...) -> Parser <std::tuple <ParserResultT <Ts> ...>>
  namespace impl
  {
    template <typename E, typename I, typename ... Ts, typename P, typename ... Ps>
    inline constexpr auto and_ (I & input, std::tuple <Ts ...> && t, P && p, Ps && ... ps) noexcept -> result::result_t <std::tuple <Ts ..., ParserResultT <P, I>, ParserResultT <Ps, I> ...>, E>
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
    return [... ps = std::move (ps)] <typename I> (I & input) constexpr noexcept
    {
      return impl::and_ <make_variant_t <ParserResultE <Ps, I> ...>> (input, std::tuple <> {}, std::move (ps) ...);
    };
  };


  // or_: (Parser <Ts> ...) -> Parser <make_variant_t <std::variant <ParserResultT <Ts> ...>>>
  namespace impl
  {
    template <typename T, typename E, typename I, typename P, typename ... Ps>
    inline constexpr auto or_ (I & input, P && p, Ps && ... ps) noexcept -> result::result_t <T, E>
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
    return [... ps = std::move (ps)] <typename I> (I & input) constexpr noexcept
    {
      return impl::or_ <make_variant_t <ParserResultT <Ps, I> ...>, make_variant_t <ParserResultE <Ps, I> ...>> (input, std::move (ps) ...);
    };
  };


  // optional: (Parser <T>) -> Parser <std::optional <T>>
  inline constexpr auto optional = [] <typename P> (P p) constexpr noexcept
  {
    return [p = std::move (p)] <typename I> (I & input) constexpr noexcept -> result::result_t <std::optional <ParserResultT <P, I>>, ParserResultE <P, I>>
    {
      auto res = p (input);
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
    return [p = std::move (p)] <typename I> (I & input) constexpr -> result::result_t <std::vector <ParserResultT <P, I>>, ParserResultE <P, I>>
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


  // repeat: (Parser <T>) -> Parser <std::vector <T>>
  inline constexpr auto more = [] <typename P> (P p) constexpr noexcept
  {
    return [p = std::move (p)] <typename I> (I & input) constexpr -> result::result_t <std::vector <ParserResultT <P, I>>, ParserResultE <P, I>>
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
      if (v.empty ())
      {
        return {};
      }
      else
      {
        return result::success {std::move (v)};
      }
    };
  };
}

#endif
