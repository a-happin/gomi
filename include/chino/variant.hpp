#ifndef CHINO_VARIANT_HPP
#define CHINO_VARIANT_HPP
#include <variant>

namespace chino
{
  template <typename To, typename From>
  inline constexpr auto variant_cast (From && v) noexcept -> To
  {
    return std::visit ([] <typename T> (T && x) constexpr noexcept -> To { return std::forward <T> (x); }, std::forward <From> (v));
  }
}

#endif

