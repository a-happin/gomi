#ifndef CHINO_TYPE_TRAITS_HPP
#define CHINO_TYPE_TRAITS_HPP
#include <type_traits>
#include <chino/type_traits/is_class_template_of.hpp>

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
