#ifndef EX_RESULT_HPP
#define EX_RESULT_HPP
#include <variant>

namespace ex
{
  template <typename T>
  struct success_t;

  template <typename E>
  struct failure_t;

  template <typename T, typename E>
  using result_t = std::variant <success_t <T>, failure_t <E>>;

  template <typename T>
  struct success_t
  {
    T value;
  };

  template <typename T>
  struct failure_t
  {
    T value;
  };

  template <typename T, typename E>
  inline constexpr auto is_success (const result_t <T, E> & x) -> bool
  {
    return std::holds_alternative <success_t <T>> (x);
  }

  template <typename T, typename E>
  inline constexpr auto is_failure (const result_t <T, E> & x) -> bool
  {
    return std::holds_alternative <failure_t <E>> (x);
  }

  template <typename U, typename T, typename E, typename F, typename G>
  requires requires (result_t <T, E> && x, F && f, G && g)
  {
    {std::forward <F> (f) (std::get <success_t <T>> (std::move (x)))} -> std::same_as <U>;
    {std::forward <G> (g) (std::get <failure_t <E>> (std::move (x)))} -> std::same_as <U>;
  }
  inline constexpr auto match (const result_t <T, E> & x, F && f, G && g) -> U
  {
    if (is_success (x))
    {
      return (std::forward <F> (f)) (std::get <success_t <T>> (std::move (x)));
    }
    else
    {
      return (std::forward <G> (g)) (std::get <failure_t <E>> (std::move (x)));
    }
  }

  template <typename U, typename T, typename E, typename F, typename G>
  requires requires (const result_t <T, E> & x, F && f, G && g)
  {
    {std::forward <F> (f) (std::get <success_t <T>> (x))} -> std::same_as <U>;
    {std::forward <G> (g) (std::get <failure_t <E>> (x))} -> std::same_as <U>;
  }
  inline constexpr auto match (const result_t <T, E> & x, F && f, G && g) -> U
  {
    if (is_success (x))
    {
      return (std::forward <F> (f)) (std::get <success_t <T>> (x));
    }
    else
    {
      return (std::forward <G> (g)) (std::get <failure_t <E>> (x));
    }
  }

  template <typename T, typename E, typename F>
  inline constexpr auto map (result_t <T, E> && x, F && f) -> result_t <decltype (std::forward <F> (f) (std::get <success_t <T>> (std::move (x)))), E>
  {
    if (is_success (x))
    {
      return success_t {std::forward <F> (f) (std::get <success_t <T>> (std::move (x)))};
    }
    else
    {
      return std::get <failure_t <E>> (x);
    }
  }

  template <typename T, typename E, typename F>
  inline constexpr auto map (const result_t <T, E> & x, F && f) -> result_t <decltype (std::forward <F> (f) (std::get <success_t <T>> (x))), E>
  {
    if (is_success (x))
    {
      return success_t {std::forward <F> (f) (std::get <success_t <T>> (x))};
    }
    else
    {
      return std::get <failure_t <E>> (x);
    }
  }

  template <typename T, typename E, typename F>
  inline constexpr auto flat_map (result_t <T, E> && x, F && f) -> decltype (std::forward <F> (f) (std::get <success_t <T>> (std::move (x))))
  {
    if (is_success (x))
    {
      return std::forward <F> (f) (std::get <success_t <T>> (std::move (x)));
    }
    else
    {
      return std::get <failure_t <E>> (x);
    }
  }

  template <typename T, typename E, typename F>
  inline constexpr auto flat_map (const result_t <T, E> & x, F && f) -> decltype (std::forward <F> (f) (std::get <success_t <T>> (x)))
  {
    if (is_success (x))
    {
      return std::forward <F> (f) (std::get <success_t <T>> (x));
    }
    else
    {
      return std::get <failure_t <E>> (x);
    }
  }
}

#endif
