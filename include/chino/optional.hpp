#ifndef CHINO_OPTIONAL_HPP
#define CHINO_OPTIONAL_HPP
#include <chino/type_traits/optional_traits.hpp>
#define RETURN(...) noexcept (noexcept (__VA_ARGS__)) -> decltype (__VA_ARGS__) { return __VA_ARGS__ ; }

namespace chino
{
  template <Optional Opt, typename F, typename G>
  inline constexpr auto match (Opt && opt, F && f, G && g) RETURN (opt ? std::forward <F> (f) (* std::forward <Opt> (opt)) : std::forward <G> (g) ())

  template <Optional Opt, typename F>
  inline constexpr auto map (Opt && opt, F && f) RETURN (opt ? std::optional {std::forward <F> (f) (* std::forward <Opt> (opt))} : std::nullopt)

  template <Optional Opt, typename F>
  inline constexpr auto flat_map (Opt && opt, F && f) RETURN (opt ? std::forward <F> (f) (* std::forward <Opt> (opt)) : std::nullopt)
}

#undef RETURN
#endif
