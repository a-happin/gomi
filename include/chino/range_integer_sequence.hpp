#ifndef CHINO_RANGE_INTEGER_SEQUENCE_HPP
#define CHINO_RANGE_INTEGER_SEQUENCE_HPP
#include <utility>

namespace chino
{
  namespace detail
  {
    template <std::integral T, T, T>
    struct range_integer_sequence;

    /* template <typename, typename> */
    /* struct concat_integer_sequence; */

    /* template <typename T, T ... xs, T ... ys> */
    /* struct concat_integer_sequence <integer_sequence  <T, xs ...>, integer_sequence <T, ys ...>> */
    /* { */
    /*   using type = integer_sequence <T, xs ..., ys ...>; */
    /* }; */

    template <typename T, typename, T, typename>
    struct twice_concat;

    template <typename T, T ... indices, T offset, T ... rest>
    struct twice_concat <T, std::integer_sequence <T, indices ...>, offset, std::integer_sequence <T, rest ...>>
    {
      using type = std::integer_sequence <T, indices ..., (indices + offset) ..., rest ...>;
    };

    template <typename T, T first, T last> requires (first + 1 > last)
    struct range_integer_sequence <T, first, last>
    {
      using type = std::integer_sequence <T>;
    };

    template <typename T, T first, T last> requires (first + 1 == last)
    struct range_integer_sequence <T, first, last>
    {
      using type = std::integer_sequence <T, first>;
    };

    template <typename T, T first, T last> requires (first + 1 < last)
    struct range_integer_sequence <T, first, last>
    {
      inline static constexpr auto mid = (first & last) + ((first ^ last) >> 1);
      /* using type = typename concat_integer_sequence < */
      /*   typename range_integer_sequence <T, first, mid>::type, */
      /*   typename range_integer_sequence <T, mid, last>::type */
      /* >::type; */
      using type = typename twice_concat <
        T,
        typename range_integer_sequence <T, first, mid>::type,
        mid - first,
        std::conditional_t <((first ^ last) & 1) != 0, std::integer_sequence <T, last - 1>, std::integer_sequence <T>>
      >::type;
    };
  }

  template <typename T, T first, T last>
  using range_integer_sequence = typename detail::range_integer_sequence <T, first, last>::type;

  template <std::size_t first, std::size_t last>
  using range_index_sequence = range_integer_sequence <std::size_t, first, last>;
}

#endif

