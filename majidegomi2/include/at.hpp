#ifndef AT_HPP
#define AT_HPP
#include <identity.hpp>
#include <integer_sequence.hpp>

namespace gomi {
  using std::size_t;
  using integer_sequence::index_sequence
      , integer_sequence::make_index_sequence;

  namespace detail {
    template <typename>
    struct at_impl;

    template <size_t ... Indices>
    struct at_impl <index_sequence <Indices ...>> {
      template <typename T>
      static auto eval (decltype (static_cast <void> (Indices)) * ... , T * , ...) -> T;
    };
  } // namespace detail

  template <size_t N , typename ... Ts>
  struct at {
    static_assert (N < sizeof ... (Ts) , "at error: out of range.");
    using type = typename decltype (detail::at_impl <make_index_sequence <N>>::eval (static_cast <Identity <Ts> *> (nullptr) ...))::type;
  };
} // namespace gomi
#endif // AT_HPP
