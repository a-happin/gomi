#ifndef CHINO_OVERLOAD_HPP
#define CHINO_OVERLOAD_HPP

namespace chino
{
  template <typename ... Ts>
  struct overload : Ts ...
  {
    using Ts::operator () ...;
  };

  // NOTE: unnecessary in C++20 or later (P1816R0)
  template <typename ... Ts>
  overload (Ts && ...) -> overload <Ts ...>;
}

#endif

