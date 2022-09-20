#ifndef CHINO_RESULT_HPP
#define CHINO_RESULT_HPP
#include <chino/type_traits/copy_cvref_from.hpp>
#include <chino/type_traits/make_variant.hpp>
/* #define RETURN(...) noexcept (noexcept (__VA_ARGS__)) -> decltype (__VA_ARGS__) { return __VA_ARGS__ ; } */

namespace chino::result
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
  using result = std::variant <success <T>, failure <E>>;

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

  template <typename T, typename E>
  struct result_traits <std::variant <failure <E>, success <T>>> : result_traits <result <T, E>>
  {
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

  template <Result T, Result U> requires (not std::is_reference_v <T>)
  inline constexpr auto result_cast (U && r) noexcept -> T
  {
    if (is_success (r))
    {
      if constexpr (std::same_as <success_type <T>, success_type <U>>)
      {
        return as_success (std::forward <U> (r));
      }
      else
      {
        return success <success_type <T>> {get_success (std::forward <U> (r))};
      }
    }
    else
    {
      if constexpr (std::same_as <failure_type <T>, failure_type <U>>)
      {
        return as_failure (std::forward <U> (r));
      }
      else
      {
        return failure <failure_type <T>> {get_failure (std::forward <U> (r))};
      }
    }
  }

  template <Result R, typename F>
  inline constexpr auto map (R && r, F && f) noexcept (noexcept (std::forward <F> (f) (get_success (std::forward <R> (r)))))
  -> result <std::remove_cvref_t <std::invoke_result_t <F, success_type <R>>>, failure_type <R>>
  {
    if (is_success (r))
    {
      return success {std::forward <F> (f) (get_success (std::forward <R> (r)))};
    }
    else
    {
      return as_failure (std::forward <R> (r));
    }
  }

  template <Result R, typename F>
  inline constexpr auto and_then (R && r, F && f) noexcept (noexcept (std::forward <F> (f) (get_success (std::forward <R> (r)))))
  -> result <
    success_type <std::invoke_result_t <F, success_type <R>>>,
    make_variant <
      failure_type <R>,
      failure_type <std::invoke_result_t <F, success_type <R>>>
    >
  >
  {
    using FR = std::invoke_result_t <F, success_type <R>>;
    using T = success_type <FR>;
    using E = make_variant <failure_type <R>, failure_type <FR>>;
    if (is_success (r))
    {
      return result_cast <result <T, E>> (std::forward <F> (f) (get_success (std::forward <R> (r))));
    }
    else
    {
      return failure <E> {get_failure (std::forward <R> (r))};
    }
  }

  template <Result R, typename F>
  inline constexpr auto catch_error (R && r, F && f) noexcept (noexcept (std::forward <F> (f) (get_failure (std::forward <R> (r)))))
  -> result <
    make_variant <
      success_type <R>,
      success_type <std::invoke_result_t <F, failure_type <R>>>
    >,
    failure_type <std::invoke_result_t <F, failure_type <R>>>
  >
  {
    using FR = std::invoke_result_t <F, failure_type <R>>;
    using T = make_variant <success_type <R>, success_type <FR>>;
    using E = failure_type <FR>;
    if (is_success (r))
    {
      return success <T> {get_success (std::forward <R> (r))};
    }
    else
    {
      return result_cast <result <T, E>> (std::forward <F> (f) (get_failure (std::forward <R> (r))));
    }
  }
}

#endif
