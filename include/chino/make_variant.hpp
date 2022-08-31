#ifndef CHINO_MAKE_VARIANT_HPP
#define CHINO_MAKE_VARIANT_HPP
#include <chino/never.hpp>
#include <variant>

namespace chino
{
  namespace detail
  {
    template <typename, typename ...>
    struct make_variant;

    template <typename T, typename ... Ts, typename ... Us>
    struct make_variant <std::variant <T, Ts ...>, Us ...>
    {
      using type = typename std::conditional_t <
        (std::is_same_v <T, never> || ... || std::is_same_v <T, Us>),
        make_variant <std::variant <Ts ...>, Us ...>,
        make_variant <std::variant <Ts ...>, Us ..., T>
      >::type;
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
  using make_variant = typename detail::make_variant <std::variant <Ts ...>>::type;

  // test
  static_assert (std::is_same_v <make_variant <int, long, long, int>, std::variant <int, long>>);
  static_assert (std::is_same_v <make_variant <int, int>, int>);
  static_assert (std::is_same_v <make_variant <int, int, never>, int>);
  static_assert (std::is_same_v <make_variant <never>, never>);
}

#endif

