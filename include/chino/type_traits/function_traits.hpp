#ifndef CHINO_TYPE_TRAITS_FUNCTION_TRAITS_HPP
#define CHINO_TYPE_TRAITS_FUNCTION_TRAITS_HPP
#include <type_traits>

namespace chino
{
  template <typename>
  struct function_traits;

  template <typename ... Ts, typename R>
  struct function_traits <auto (Ts ...) -> R>
  {
    using Args = std::tuple <Ts ...>;
    using Result = R;
  };
}

#endif
