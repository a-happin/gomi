#ifndef BOOL_HPP
#define BOOL_HPP
#include <type_traits>

namespace gomi {
  namespace detail {
    template <typename ... Types>
    struct nand_ : std::true_type {};

    template <typename ... Types>
    struct nand_ <Types * ...> : std::false_type {};
  } // namespace detail

  template <bool ... Bs>
  using nand_ = typename detail::nand_ <
    typename std::conditional <Bs , int * , int>::type ...
  >::type;

  template <bool B>
  using not_ = nand_ <B , B>;

  template <bool ... Bs>
  using and_ = not_ <nand_ <Bs ...>::value>;

  template <bool ...Bs>
  using or_ = nand_ <not_ <Bs>::value ...>;

  template <bool ...Bs>
  using nor_ = not_ <or_ <Bs...>::value>;
} // namespace gomi
#endif // BOOL_HPP
