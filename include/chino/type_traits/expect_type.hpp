#ifndef CHINO_TYPE_TRAITS_EXPECT_TYPE_HPP
#define CHINO_TYPE_TRAITS_EXPECT_TYPE_HPP
#include <type_traits>

namespace chino
{
  template <typename T, typename U>
  inline constexpr auto expect_type (const U &) -> void
  {
    static_assert (std::is_same_v <T, U>);
  }
}

#endif
