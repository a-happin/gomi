#ifndef CHINO_PARSER_RESULT_HPP
#define CHINO_PARSER_RESULT_HPP
#include <chino/never.hpp>
#include <chino/type_traits/copy_cvref_from.hpp>
#include <chino/type_traits/make_variant.hpp>

namespace chino::parser::result
{
  // ********************************
  // * define result types
  // ********************************
  struct undefined
  {
    friend constexpr auto operator <=> (undefined, undefined) noexcept -> std::strong_ordering = default;
  };

  template <typename T>
  struct success
  {
    [[no_unique_address]] T value;

    template <typename U>
    constexpr operator U () const noexcept requires std::same_as <T, never>
    {
      return unreachable <U> ();
    }
  };

  template <typename T>
  success (T &&) -> success <std::remove_cvref_t <T>>;

  template <typename E>
  struct failure
  {
    [[no_unique_address]] E error;

    template <typename U>
    constexpr operator U () const noexcept requires std::same_as <E, never>
    {
      return unreachable <U> ();
    }
  };

  template <typename E>
  failure (E &&) -> failure <std::remove_cvref_t <E>>;


  // ********************************
  // * utility result types
  // ********************************
  template <typename U, typename T, typename E>
  using result3 = make_variant <
    U,
    std::conditional_t <std::same_as <T, never>, never, success <T>>,
    std::conditional_t <std::same_as <E, never>, never, failure <E>>
  >;

  template <typename T, typename E>
  using result2 = result3 <never, T, E>;

  template <typename T>
  using optional = std::variant <undefined, success <T>>;


  // ********************************
  // * result traits
  // ********************************
  template <typename>
  struct result_traits;

  template <typename U, typename T, typename E>
  struct result_traits <std::variant <U, success <T>, failure <E>>>
  {
    using undefined_type = U;
    using success_type = T;
    using failure_type = E;

    template <typename R>
    static constexpr auto is_undefined (const R & r) noexcept
    {
      return std::holds_alternative <U> (r);
    }
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
    static constexpr auto as_undefined (R && r) noexcept -> decltype (auto)
    {
      return std::get <U> (std::forward <R> (r));
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

  template <typename T, typename E>
  struct result_traits <std::variant <success <T>, failure <E>>>
  {
    using undefined_type = never;
    using success_type = T;
    using failure_type = E;

    template <typename R>
    static constexpr auto is_undefined (const R &) noexcept
    {
      return false;
    }
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
    static constexpr auto as_undefined (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <never, R &&>> ();
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
  struct result_traits <std::variant <undefined, success <T>>>
  {
    using undefined_type = undefined;
    using success_type = T;
    using failure_type = never;

    template <typename R>
    static constexpr auto is_undefined (const R & r) noexcept
    {
      return std::holds_alternative <undefined> (r);
    }
    template <typename R>
    static constexpr auto is_success (const R & r) noexcept
    {
      return std::holds_alternative <success <T>> (r);
    }
    template <typename R>
    static constexpr auto is_failure (const R &) noexcept
    {
      return false;
    }
    template <typename R>
    static constexpr auto as_undefined (R && r) noexcept -> decltype (auto)
    {
      return std::get <undefined> (std::forward <R> (r));
    }
    template <typename R>
    static constexpr auto as_success (R && r) noexcept -> decltype (auto)
    {
      return std::get <success <T>> (std::forward <R> (r));
    }
    template <typename R>
    static constexpr auto as_failure (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <failure <never>, R &&>> ();
    }
  };

  template <typename E>
  struct result_traits <std::variant <undefined, failure <E>>>
  {
    using undefined_type = undefined;
    using success_type = never;
    using failure_type = E;

    template <typename R>
    static constexpr auto is_undefined (const R & r) noexcept
    {
      return std::holds_alternative <undefined> (r);
    }
    template <typename R>
    static constexpr auto is_success (const R &) noexcept
    {
      return false;
    }
    template <typename R>
    static constexpr auto is_failure (const R & r) noexcept
    {
      return std::holds_alternative <failure <E>> (r);
    }
    template <typename R>
    static constexpr auto as_undefined (R && r) noexcept -> decltype (auto)
    {
      return std::get <undefined> (std::forward <R> (r));
    }
    template <typename R>
    static constexpr auto as_success (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <success <never>, R &&>> ();
    }
    template <typename R>
    static constexpr auto as_failure (R && r) noexcept -> decltype (auto)
    {
      return std::get <failure <E>> (std::forward <R> (r));
    }
  };

  template <>
  struct result_traits <undefined>
  {
    using undefined_type = undefined;
    using success_type = never;
    using failure_type = never;

    template <typename R>
    static constexpr auto is_undefined (const R &) noexcept
    {
      return true;
    }
    template <typename R>
    static constexpr auto is_success (const R &) noexcept
    {
      return false;
    }
    template <typename R>
    static constexpr auto is_failure (const R &) noexcept
    {
      return false;
    }
    template <typename R>
    static constexpr auto as_undefined (R && r) noexcept -> decltype (auto)
    {
      return std::forward <R> (r);
    }
    template <typename R>
    static constexpr auto as_success (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <success <never>, R &&>> ();
    }
    template <typename R>
    static constexpr auto as_failure (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <failure <never>, R &&>> ();
    }
  };

  template <typename T>
  struct result_traits <success <T>>
  {
    using undefined_type = never;
    using success_type = T;
    using failure_type = never;

    template <typename R>
    static constexpr auto is_undefined (const R &) noexcept
    {
      return false;
    }
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
    static constexpr auto as_undefined (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <never, R &&>> ();
    }
    template <typename R>
    static constexpr auto as_success (R && r) noexcept -> decltype (auto)
    {
      return std::forward <R> (r);
    }
    template <typename R>
    static constexpr auto as_failure (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <failure <never>, R &&>> ();
    }
  };

  template <typename E>
  struct result_traits <failure <E>>
  {
    using undefined_type = never;
    using success_type = never;
    using failure_type = E;

    template <typename R>
    static constexpr auto is_undefined (const R &) noexcept
    {
      return false;
    }
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
    static constexpr auto as_undefined (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <never, R &&>> ();
    }
    template <typename R>
    static constexpr auto as_success (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <success <never>, R &&>> ();
    }
    template <typename R>
    static constexpr auto as_failure (R && r) noexcept -> decltype (auto)
    {
      return std::forward <R> (r);
    }
  };

  template <>
  struct result_traits <never>
  {
    using undefined_type = never;
    using success_type = never;
    using failure_type = never;

    template <typename R>
    static constexpr auto is_undefined (const R &) noexcept
    {
      return false;
    }
    template <typename R>
    static constexpr auto is_success (const R &) noexcept
    {
      return false;
    }
    template <typename R>
    static constexpr auto is_failure (const R &) noexcept
    {
      return false;
    }
    template <typename R>
    static constexpr auto as_undefined (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <never, R &&>> ();
    }
    template <typename R>
    static constexpr auto as_success (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <success <never>, R &&>> ();
    }
    template <typename R>
    static constexpr auto as_failure (R &&) noexcept -> decltype (auto)
    {
      return unreachable <copy_cvref_from <failure <never>, R &&>> ();
    }
  };


  // ********************************
  // * utility functions
  // ********************************
  template <typename T>
  concept Result = requires
  {
    typename result_traits <std::remove_cvref_t <T>>::undefined_type;
    typename result_traits <std::remove_cvref_t <T>>::success_type;
    typename result_traits <std::remove_cvref_t <T>>::failure_type;
  };

  template <Result R>
  using undefined_type = typename result_traits <std::remove_cvref_t <R>>::undefined_type;

  template <Result R>
  using success_type = typename result_traits <std::remove_cvref_t <R>>::success_type;

  template <Result R>
  using failure_type = typename result_traits <std::remove_cvref_t <R>>::failure_type;

  template <Result R>
  inline constexpr auto is_undefined (const R & r) noexcept
  {
    return result_traits <std::remove_cvref_t <R>>::is_undefined (r);
  }

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
  inline constexpr auto as_undefined (R && r) noexcept -> decltype (auto)
  {
    return result_traits <std::remove_cvref_t <R>>::as_undefined (std::forward <R> (r));
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

  template <Result T, Result R> requires (not std::is_reference_v <T>)
  inline constexpr auto result_cast (R && r) noexcept -> T
  {
    if (is_success (r))
    {
      if constexpr (std::same_as <success_type <T>, success_type <R>>)
      {
        return as_success (std::forward <R> (r));
      }
      else
      {
        return success <success_type <T>> {get_success (std::forward <R> (r))};
      }
    }
    else if (is_failure (r))
    {
      if constexpr (std::same_as <failure_type <T>, failure_type <R>>)
      {
        return as_failure (std::forward <R> (r));
      }
      else
      {
        return failure <failure_type <T>> {get_failure (std::forward <R> (r))};
      }
    }
    else
    {
      return as_undefined (std::forward <R> (r));
    }
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
  inline constexpr auto map (R && r, F && f) noexcept (noexcept (std::forward <F> (f) (get_success (std::forward <R> (r)))))
  -> result3 <
    undefined_type <R>,
    std::remove_cvref_t <std::invoke_result_t <F, success_type <R>>>,
    failure_type <R>
  >
  {
    if (is_success (r))
    {
      return success {std::forward <F> (f) (get_success (std::forward <R> (r)))};
    }
    else if (is_failure (r))
    {
      return as_failure (std::forward <R> (r));
    }
    else
    {
      return as_undefined (std::forward <R> (r));
    }
  }

  template <Result R, typename F>
  inline constexpr auto and_then (R && r, F && f) noexcept (noexcept (std::forward <F> (f) (get_success (std::forward <R> (r)))))
  -> result3 <
    make_variant <
      undefined_type <R>,
      undefined_type <std::invoke_result_t <F, success_type <R>>>
    >,
    success_type <std::invoke_result_t <F, success_type <R>>>,
    make_variant <
      failure_type <R>,
      failure_type <std::invoke_result_t <F, success_type <R>>>
    >
  >
  {
    using FR = std::invoke_result_t <F, success_type <R>>;
    using U = make_variant <undefined_type <R>, undefined_type <FR>>;
    using T = success_type <FR>;
    using E = make_variant <failure_type <R>, failure_type <FR>>;
    if (is_success (r))
    {
      if constexpr (std::same_as <failure_type <FR>, E> && std::same_as <undefined_type <FR>, U>)
      {
        return std::forward <F> (f) (get_success (std::forward <R> (r)));
      }
      else
      {
        return result_cast <result3 <U, T, E>> (std::forward <F> (f) (get_success (std::forward <R> (r))));
      }
    }
    else if (is_failure (r))
    {
      return failure <E> {get_failure (std::forward <R> (r))};
      /* return as_failure (std::forward <R> (r)); */
    }
    else
    {
      return as_undefined (std::forward <R> (r));
    }
  }

  template <Result R, typename F>
  inline constexpr auto catch_error (R && r, F && f) noexcept (noexcept (std::forward <F> (f) (get_failure (std::forward <R> (r)))))
  -> result3 <
    make_variant <
      undefined_type <R>,
      undefined_type <std::invoke_result_t <F, failure_type <R>>>
    >,
    make_variant <
      success_type <R>,
      success_type <std::invoke_result_t <F, failure_type <R>>>
    >,
    failure_type <std::invoke_result_t <F, failure_type <R>>>
  >
  {
    using FR = std::invoke_result_t <F, failure_type <R>>;
    using U = make_variant <undefined_type <R>, undefined_type <FR>>;
    using T = make_variant <success_type <R>, success_type <FR>>;
    using E = failure_type <FR>;
    if (is_success (r))
    {
      return success <T> {get_success (std::forward <R> (r))};
      /* return as_success (std::forward <R> (r)); */
    }
    else if (is_failure (r))
    {
      if constexpr (std::same_as <success_type <FR>, T> && std::same_as <undefined_type <FR>, U>)
      {
        return std::forward <F> (f) (get_failure (std::forward <R> (r)));
      }
      else
      {
        return result_cast <result3 <U, T, E>> (std::forward <F> (f) (get_failure (std::forward <R> (r))));
      }
    }
    else
    {
      return as_undefined (std::forward <R> (r));
    }
  }
}

#endif
