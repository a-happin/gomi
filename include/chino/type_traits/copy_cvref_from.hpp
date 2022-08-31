#ifndef CHINO_TYPE_TRAITS_COPY_CVREF_FROM_HPP
#define CHINO_TYPE_TRAITS_COPY_CVREF_FROM_HPP
#include <type_traits>

namespace chino
{
  namespace detail
  {
    template <bool cond, template <typename> class F, typename T>
    using apply_if = std::conditional_t <cond, F <T>, T>;
  }
  template <typename T, typename U>
  using copy_cvref_from = detail::apply_if <std::is_rvalue_reference_v <U>, std::add_rvalue_reference_t,
    detail::apply_if <std::is_lvalue_reference_v <U>, std::add_lvalue_reference_t,
      detail::apply_if <std::is_volatile_v <std::remove_reference_t <U>>, std::add_volatile_t,
        detail::apply_if <std::is_const_v <std::remove_reference_t <U>>, std::add_const_t, std::remove_cvref_t <T>>
      >
    >
  >;

  static_assert (std::is_same_v <copy_cvref_from <int, long &>, int &>);
  static_assert (std::is_same_v <copy_cvref_from <int, long &&>, int &&>);
  static_assert (std::is_same_v <copy_cvref_from <int, const long &>, const int &>);
  static_assert (std::is_same_v <copy_cvref_from <int, const long &&>, const int &&>);
  static_assert (std::is_same_v <copy_cvref_from <int, volatile long>, volatile int>);
}

#endif

