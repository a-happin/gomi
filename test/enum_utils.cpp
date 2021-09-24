#include <enum_utils.hpp>

namespace ns1
{
  enum class A : int
  {
  };
  ENUM_UTILS_USING_OPERATORS;
}

namespace ns2
{
  enum class A : unsigned long long
  {
  };
  ENUM_UTILS_USING_OPERATORS;
}

namespace ns3
{
  enum class A : char
  {
  };
  ENUM_UTILS_USING_OPERATORS;
}

namespace enum_utils::operators
{
  template <>
  inline constexpr auto provider_v <ns1::A> = PROVIDE_BITWISE_OPERATORS;

  template <>
  inline constexpr auto provider_v <ns2::A> = PROVIDE_ARITHMETIC_OPERATORS;

  template <>
  inline constexpr auto provider_v <ns3::A> = PROVIDE_SHIFT_OPERATIRS;
}


template <typename EnumType, typename UnderlyingType = std::underlying_type_t <EnumType>, typename F>
requires requires (F f, EnumType x, EnumType y)
{
  {f (x, y)};
}
constexpr auto test (F f) noexcept
{
  for (UnderlyingType x = 1; x <= 10; ++ x)
  {
    for (UnderlyingType y = 1; y <= 10; ++ y)
    {
      EnumType ex {x}, ey {y};
      auto x_ = x;
      if (f (ex, ey) != EnumType {f (x_, y)})
      {
        return false;
      }
      if (ex != EnumType {x_})
      {
        return false;
      }
    }
  }
  return true;
}

template <typename EnumType, typename UnderlyingType = std::underlying_type_t <EnumType>, typename F>
requires requires (F f, EnumType x)
{
  {f (x)};
}
constexpr auto test (F f) noexcept
{
  for (UnderlyingType x = 1; x <= 10; ++ x)
  {
    EnumType ex {x};
    auto x_ = x;
    if (f (ex) != EnumType {f (x_)})
    {
      return false;
    }
    if (ex != EnumType {x_})
    {
      return false;
    }
  }
  return true;
}

template <typename EnumType, typename UnderlyingType = std::underlying_type_t <EnumType>, typename F>
requires requires (F f, EnumType x, std::size_t i)
{
  {f (x, i)};
}
constexpr auto test_shift (F f) noexcept
{
  for (UnderlyingType x = 1; x <= 10; ++ x)
  {
    for (std::size_t y = 1; y < 10; ++ y)
    {
      EnumType ex {x};
      auto x_ = x;
      if (f (ex, y) != EnumType {static_cast <UnderlyingType> (f (x_, y))})
      {
        return false;
      }
    }
  }
  return true;
}

auto main () -> int
{
  static_assert ([] () constexpr {
    using ns1::A;
    return test <A> ([] (auto x, auto y) constexpr noexcept { return x & y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x &= y; })
        && test <A> ([] (auto x, auto y) constexpr noexcept { return x | y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x |= y; })
        && test <A> ([] (auto x, auto y) constexpr noexcept { return x ^ y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x ^= y; })
        && test <A> ([] (auto x) constexpr noexcept { return ~ x; })
        ;
  } ());

  static_assert ([] () constexpr {
    using ns2::A;
    return test <A> ([] (auto x, auto y) constexpr noexcept { return x + y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x += y; })
        && test <A> ([] (auto x, auto y) constexpr noexcept { return x - y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x -= y; })
        && test <A> ([] (auto x, auto y) constexpr noexcept { return x * y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x *= y; })
        && test <A> ([] (auto x, auto y) constexpr noexcept { return x / y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x /= y; })
        && test <A> ([] (auto x, auto y) constexpr noexcept { return x % y; })
        && test <A> ([] (auto & x, auto y) constexpr noexcept { return x %= y; })
        && test <A> ([] (auto x) constexpr noexcept { return + x; })
        && test <A> ([] (auto x) constexpr noexcept { return - x; })
        && test <A> ([] (auto & x) constexpr noexcept { return ++ x; })
        && test <A> ([] (auto & x) constexpr noexcept { return -- x; })
        && test <A> ([] (auto & x) constexpr noexcept { return x ++; })
        && test <A> ([] (auto & x) constexpr noexcept { return x --; })
        ;
  } ());

  static_assert ([] () constexpr {
    using ns3::A;
    A a {1};
    a <<= 1;
    return test_shift <A> ([] (auto x, auto y) constexpr noexcept { return x << y; })
        && test_shift <A> ([] (auto & x, auto y) constexpr noexcept { return x <<= y; })
        && test_shift <A> ([] (auto x, auto y) constexpr noexcept { return x >> y; })
        && test_shift <A> ([] (auto & x, auto y) constexpr noexcept { return x >>= y; })
        ;
  } ());
}
