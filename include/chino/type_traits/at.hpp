#ifndef CHINO_TYPE_TRAITS_AT_HPP
#define CHINO_TYPE_TRAITS_AT_HPP
#include <utility>

namespace chino
{
  namespace detail
  {
    template <typename>
    struct at_impl;

    template <std::size_t ... indices>
    struct at_impl <std::index_sequence <indices ...>>
    {
      template <typename T>
      constexpr auto operator () (std::void_t <decltype (indices)> * ..., T *, ...) noexcept -> T;
    };
  }

  template <std::size_t N, typename ... Ts>
  struct at
  {
    using type = typename std::invoke_result_t <detail::at_impl <std::make_index_sequence <N>>, std::type_identity <Ts> * ...>::type;
  };

  template <std::size_t N, typename ... Ts>
  using at_t = typename at <N, Ts ...>::type;
}

#endif
