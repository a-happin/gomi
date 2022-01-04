#ifndef CHINO_TYPE_TRAITS_OPTIONAL_TRAITS_HPP
#define CHINO_TYPE_TRAITS_OPTIONAL_TRAITS_HPP
#include <type_traits>
#include <optional>

namespace chino
{
  template <typename>
  struct optional_traits;

  template <typename T>
  struct optional_traits <std::optional <T>>
  {
    using value_type = T;
  };

  template <typename T>
  concept Optional = requires
  {
    typename optional_traits <std::remove_cvref_t <T>>::value_type;
  };
}
#endif
