#ifndef CHINO_TYPE_TRAITS_SIMILAR_TO_HPP
#define CHINO_TYPE_TRAITS_SIMILAR_TO_HPP
#include <concepts>

namespace chino
{
  template <typename T, typename U>
  concept similar_to = std::same_as <std::remove_cvref_t <T>, std::remove_cvref_t <U>>;
}
#endif
