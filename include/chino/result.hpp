#ifndef CHINO_RESULT_HPP
#define CHINO_RESULT_HPP
#include <variant>
#define RETURN(...) noexcept (noexcept (__VA_ARGS__)) -> decltype (__VA_ARGS__) { return __VA_ARGS__ ; }

namespace chino
{
  template <typename T>
  struct success_t
  {
    T value;
  };

  template <typename T>
  struct failure_t
  {
    T error;
  };

  namespace result_traits
  {
    template <typename>
    struct result_traits;

    template <typename T, typename E>
    struct result_traits <std::variant <success_t <T>, failure_t <E>>>
    {
      using success_type = T;
      using failure_type = E;
    };

    template <typename T, typename E>
    struct result_traits <std::variant <failure_t <E>, success_t <T>>>
    {
      using success_type = T;
      using failure_type = E;
    };

    template <typename R>
    using success_type = typename result_traits <std::remove_cvref_t <R>>::success_type;

    template <typename R>
    using failure_type = typename result_traits <std::remove_cvref_t <R>>::failure_type;
  }

  template <typename T, typename E>
  using result_t = std::variant <success_t <T>, failure_t <E>>;

  template <typename T>
  concept Result = requires
  {
    typename result_traits::success_type <T>;
    typename result_traits::failure_type <T>;
  };

  template <typename T>
  inline constexpr auto success (T && x) RETURN (success_t <std::remove_cvref_t <T>> {std::forward <T> (x)})

  template <typename T>
  inline constexpr auto failure (T && x) RETURN (failure_t <std::remove_cvref_t <T>> {std::forward <T> (x)})

  template <Result R>
  inline constexpr auto is_success (const R & x) noexcept -> bool
  {
    return std::holds_alternative <success_t <result_traits::success_type <R>>> (x);
  }

  template <Result R>
  inline constexpr auto is_failure (const R & x) noexcept -> bool
  {
    return std::holds_alternative <failure_t <result_traits::failure_type <R>>> (x);
  }

  template <Result R>
  inline constexpr auto get_success (R && x) -> decltype (auto)
  {
    return (std::get <success_t <result_traits::success_type <R>>> (std::forward <R> (x)).value);
  }

  template <Result R>
  inline constexpr auto get_failure (R && x) -> decltype (auto)
  {
    return (std::get <failure_t <result_traits::failure_type <R>>> (std::forward <R> (x)).error);
  }

  template <Result R>
  inline constexpr auto unwrap (R && x) -> decltype (auto)
  {
    if (is_success (x))
    {
      return get_success (std::forward <R> (x));
    }
    else
    {
      throw get_failure (std::forward <R> (x));
    }
  }

  template <Result R, typename F, typename G>
  inline constexpr auto match (R && x, F && f, G && g) RETURN (is_success (x) ? std::forward <F> (f) (get_success (std::forward <R> (x))) : std::forward <G> (g) (get_failure (std::forward <R> (x))))

  template <Result R, typename F>
  inline constexpr auto map (R && x, F && f) -> result_t <std::remove_cvref_t <decltype (std::forward <F> (f) (get_success (std::forward <R> (x))))>, result_traits::failure_type <R>>
  {
    if (is_success (x))
    {
      return success (std::forward <F> (f) (get_success (std::forward <R> (x))));
    }
    else
    {
      return failure (get_failure (std::forward <R> (x)));
    }
  }

  // f: result_traits::success_type<R> => success_t<U> のとき、コンパイルエラー(substitution failure) になるが、それはmapを使えばいい話なのでhelperは書かないでおく
  template <Result R, typename F>
  inline constexpr auto flat_map (R && x, F && f) RETURN (is_success (x) ? std::forward <F> (f) (get_success (std::forward <R> (x))) : failure (get_failure (std::forward <R> (x))))
}

#undef RETURN
#endif
