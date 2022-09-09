#ifndef CHINO_OSTREAM_JOIN_HPP
#define CHINO_OSTREAM_JOIN_HPP
#include <iterator>
#include <ranges>

namespace chino
{
  template <std::input_iterator I, std::sentinel_for <I> S, typename Stream, typename Delimiter>
  inline constexpr auto ostream_join (I ite, S last, Stream & stream, const Delimiter & delimiter) -> decltype (auto)
  {
    if (ite != last) stream << * ite ++;
    while (ite != last) stream << delimiter << * ite ++;
    return stream;
  }

  template <std::ranges::input_range R, typename Stream, typename Delimiter>
  inline constexpr auto ostream_join (R && r, Stream & stream, const Delimiter & delimiter) -> decltype (auto)
  {
    return ostream_join (std::ranges::begin (r), std::ranges::end (r), stream, delimiter);
  }
}

#endif

