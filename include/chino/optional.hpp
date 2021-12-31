#ifndef CHINO_OPTIONAL_HPP
#define CHINO_OPTIONAL_HPP
#include <optional>
#include <chino/type_traits/is_class_template_of.hpp>
#define RETURN(...) decltype (__VA_ARGS__) { return __VA_ARGS__ ; }

namespace chino
{

  template <typename T>
  concept Optional = class_template_of <T, std::optional>;

  template <typename U, Optional Opt, typename T, typename F, typename G>
  requires requires (Opt && opt, F && f, G && g)
  {
    {std::forward <F> (f) (* std::forward <Opt> (opt))} -> std::convertible_to <U>;
    {std::forward <G> (g) ()} -> std::convertible_to <U>;
  }
  inline constexpr auto match (Opt & opt, F && f, G && g)
  {
    if (opt)
    {
      return std::forward <F> (f) (* std::forward <Opt> (opt));
    }
    else
    {
      return std::forward <G> (g) ();
    }
  }

  template <Optional Opt, typename T, typename F>
  inline constexpr auto map (Opt && opt, F && f) -> std::optional <decltype (std::forward <F> (f) (* std::forward <Opt> (opt)))>
  {
    if (opt)
    {
      return std::optional {std::forward <F> (f) (* std::forward <Opt> (opt))};
    }
    return {};
  }

  template <Optional Opt, typename F>
  inline constexpr auto flat_map (Opt && opt, F && f) -> decltype (std::forward <F> (f) (* std::forward <Opt> (opt)))
  {
    if (opt)
    {
      return std::forward <F> (f) (* std::forward <Opt> (opt));
    }
    return {};
  }
}

#undef RETURN
#endif
