#ifndef INTEGER_SEQUENCE_HPP
#define INTEGER_SEQUENCE_HPP
#include <type_traits>

namespace gomi {
  using size_t = decltype (sizeof (0));

  template <typename T , T ... Indices>
  struct integer_sequence {};

  template <size_t ... Indices>
  using index_sequence = integer_sequence <size_t , Indices ...>;

  namespace detail {
    template <typename T , T , T , typename = void>
    struct range_integer_sequence;

    template <typename , typename>
    struct twice_cat;

    template <typename T , T ... Indices , T ... Rest>
    struct twice_cat <integer_sequence <T , Indices ...> , integer_sequence <T , Rest ...>> {
      using type = integer_sequence <T , Indices ... , (Indices + sizeof ... (Indices)) ... , Rest ...>;
    };

    template <typename T , T begin , T end>
    struct range_integer_sequence <T , begin , end ,
      std::enable_if_t <(begin + 1 < end && (end - begin) % 2 == 0)>> {
        using type = typename twice_cat <
          typename range_integer_sequence <T , begin , (begin + end) / 2>::type ,
          integer_sequence <T>
        >::type;
    };

    template <typename T , T begin , T end>
    struct range_integer_sequence <T , begin , end ,
      std::enable_if_t <(begin + 1 < end && (end - begin) % 2 == 1)>> {
        using type = typename twice_cat <
          typename range_integer_sequence <T , begin , (begin + end) / 2>::type ,
          integer_sequence <T , end - 1>
        >::type;
    };

    template <typename T , T begin , T end>
    struct range_integer_sequence <T , begin , end ,
      std::enable_if_t <(begin + 1 == end)>> {
        using type = integer_sequence <T , begin>;
    };

    template <typename T , T begin , T end>
    struct range_integer_sequence <T , begin , end ,
      std::enable_if_t <(begin + 1 > end)>> {
        using type = integer_sequence <T>;
    };
  } // namespace detail

  template <typename T , T begin , T end>
  using range_integer_sequence = typename detail::range_integer_sequence <T , begin , end>::type;

  template <size_t begin , size_t end>
  using range_index_sequence = range_integer_sequence <size_t , begin , end>;

  template <typename T , T N>
  using make_integer_sequence = range_integer_sequence <T , static_cast <T> (0) , N>;

  template <size_t N>
  using make_index_sequence = make_integer_sequence <size_t , N>;

  template <typename ... Types>
  using index_sequence_for = make_index_sequence <sizeof ... (Types)>;
} // namespace gomi
#endif // INTEGER_SEQUENCE_HPP
