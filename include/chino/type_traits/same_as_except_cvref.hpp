#ifndef CHINO_TYPE_TRAITS_SAME_AS_EXCEPT_CVREF_HPP
#define CHINO_TYPE_TRAITS_SAME_AS_EXCEPT_CVREF_HPP
#include <concepts>

namespace chino
{
  template <typename T, typename U>
  concept same_as_except_cvref = std::same_as <std::remove_cvref_t <T>, std::remove_cvref_t <U>>;
}
#endif
