#ifndef UTIL_ENUM_UTILS_HPP
#define UTIL_ENUM_UTILS_HPP
#include <type_traits>
#include <concepts>
#include <cstdint>

#define ENUM_UTILS_USING_ENUM(scope, v) inline constexpr auto v = scope::v
#define ENUM_UTILS_DEFINE_ENUM_OPERATOR2(ope, flag)                                             \
  template <Enum T>                                                                             \
  requires (bittest (provider_v <T>, flag))                                                     \
  inline constexpr auto operator ope (T lhs, T rhs) noexcept -> T                               \
  {                                                                                             \
    using U = std::underlying_type_t <T>;                                                       \
    return static_cast <T> (static_cast <U> (static_cast <U> (lhs) ope static_cast <U> (rhs))); \
  }                                                                                             \
  template <Enum T>                                                                             \
  requires (bittest (provider_v <T>, flag))                                                     \
  inline constexpr auto operator ope ## = (T & lhs, T rhs) noexcept -> T &                      \
  {                                                                                             \
    return lhs = lhs ope rhs;                                                                   \
  }
#define ENUM_UTILS_DEFINE_ENUM_SHIFT_OPERATOR(ope, flag)                         \
  template <Enum T, std::integral IntType>                                       \
  requires (bittest (provider_v <T>, flag))                                      \
  inline constexpr auto operator ope (T lhs, IntType rhs) noexcept -> T          \
  {                                                                              \
    using U = std::underlying_type_t <T>;                                        \
    return static_cast <T> (static_cast <U> (static_cast <U> (lhs) ope rhs));    \
  }                                                                              \
  template <Enum T, std::integral IntType>                                       \
  requires (bittest (provider_v <T>, flag))                                      \
  inline constexpr auto operator ope ## = (T & lhs, IntType rhs) noexcept -> T & \
  {                                                                              \
    return lhs = lhs ope rhs;                                                    \
  }
#define ENUM_UTILS_DEFINE_ENUM_UNARY_OPERATOR(ope, flag)                \
  template <Enum T>                                                     \
  requires (bittest (provider_v <T>, flag))                             \
  inline constexpr auto operator ope (T x) noexcept -> T                \
  {                                                                     \
    using U = std::underlying_type_t <T>;                               \
    return static_cast <T> (static_cast <U> (ope static_cast <U> (x))); \
  }
#define ENUM_UTILS_DEFINE_ENUM_INCREMENT_OPERATOR(ope, flag)    \
  template <Enum T>                                             \
  requires (bittest (provider_v <T>, flag))                     \
  inline constexpr auto operator ope (T & x) noexcept -> T &    \
  {                                                             \
    using U = std::underlying_type_t <T>;                       \
    auto tmp = static_cast <U> (x);                             \
    return x = static_cast <T> (ope tmp);                       \
  }                                                             \
  template <Enum T>                                             \
  requires (bittest (provider_v <T>, flag))                     \
  inline constexpr auto operator ope (T & x, int) noexcept -> T \
  {                                                             \
    auto tmp = x;                                               \
    ope x;                                                      \
    return tmp;                                                 \
  }
#define ENUM_UTILS_USING_OPERATORS                \
  using enum_utils::operators::operator &   \
      , enum_utils::operators::operator &=  \
      , enum_utils::operators::operator |   \
      , enum_utils::operators::operator |=  \
      , enum_utils::operators::operator ^   \
      , enum_utils::operators::operator ^=  \
      , enum_utils::operators::operator ~   \
      , enum_utils::operators::operator <<  \
      , enum_utils::operators::operator <<= \
      , enum_utils::operators::operator >>  \
      , enum_utils::operators::operator >>= \
      , enum_utils::operators::operator +   \
      , enum_utils::operators::operator +=  \
      , enum_utils::operators::operator -   \
      , enum_utils::operators::operator -=  \
      , enum_utils::operators::operator *   \
      , enum_utils::operators::operator *=  \
      , enum_utils::operators::operator /   \
      , enum_utils::operators::operator /=  \
      , enum_utils::operators::operator %   \
      , enum_utils::operators::operator %=  \
      , enum_utils::operators::operator ++  \
      , enum_utils::operators::operator --  \
      , enum_utils::operators::bittest

namespace enum_utils
{
  using std::size_t;

  template <typename T>
  concept Enum = std::is_enum_v <T>;

  namespace operators
  {
    enum class provide_operators_flag_t : std::uint_fast16_t
    {
      PROVIDE_NONE                 = 0u,

      PROVIDE_AND                  = 1u,
      PROVIDE_OR                   = 1u << 1,
      PROVIDE_XOR                  = 1u << 2,
      PROVIDE_NOT                  = 1u << 3,

      PROVIDE_SHIFT_LEFT           = 1u << 4,
      PROVIDE_SHIFT_RIGHT          = 1u << 5,

      PROVIDE_ADDITION             = 1u << 6,
      PROVIDE_SUBTRACTION          = 1u << 7,
      PROVIDE_MULTIPLICATION       = 1u << 8,
      PROVIDE_DIVISION             = 1u << 9,
      PROVIDE_MODULO               = 1u << 10,

      PROVIDE_UNARY_PLUS           = 1u << 11,
      PROVIDE_UNARY_MINUS          = 1u << 12,

      PROVIDE_INCREMENT            = 1u << 13,
      PROVIDE_DECREMENT            = 1u << 14,

      /* PROVIDE_TO_BOOL              = 1u << 15, */ // cannot

      PROVIDE_BITWISE_OPERATORS    = PROVIDE_AND | PROVIDE_OR | PROVIDE_XOR | PROVIDE_NOT,
      PROVIDE_SHIFT_OPERATIRS      = PROVIDE_SHIFT_LEFT | PROVIDE_SHIFT_RIGHT,
      PROVIDE_ARITHMETIC_OPERATORS = PROVIDE_ADDITION | PROVIDE_SUBTRACTION | PROVIDE_MULTIPLICATION | PROVIDE_DIVISION | PROVIDE_MODULO | PROVIDE_UNARY_PLUS | PROVIDE_UNARY_MINUS | PROVIDE_INCREMENT | PROVIDE_DECREMENT,
    };

    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_NONE);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_AND);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_OR);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_XOR);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_NOT);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_SHIFT_LEFT);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_SHIFT_RIGHT);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_ADDITION);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_SUBTRACTION);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_MULTIPLICATION);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_DIVISION);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_MODULO);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_UNARY_PLUS);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_UNARY_MINUS);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_INCREMENT);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_DECREMENT);

    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_BITWISE_OPERATORS);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_SHIFT_OPERATIRS);
    ENUM_UTILS_USING_ENUM (provide_operators_flag_t, PROVIDE_ARITHMETIC_OPERATORS);

    template <typename T>
    inline constexpr auto provider_v = PROVIDE_NONE;

    template <>
    inline constexpr auto provider_v <provide_operators_flag_t> = PROVIDE_BITWISE_OPERATORS;

    // bittest
    template <Enum T>
    requires ((static_cast <std::underlying_type_t <T>> (provider_v <T>) & static_cast <std::underlying_type_t <T>> (PROVIDE_AND)) != 0)
    inline constexpr auto bittest (const T & lhs, const T & rhs) noexcept -> bool
    {
      using U = std::underlying_type_t <T>;
      return (static_cast <U> (lhs) & static_cast <U> (rhs)) != 0;
    }

    // operators
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (&, PROVIDE_AND)
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (|, PROVIDE_OR)
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (^, PROVIDE_XOR)
    ENUM_UTILS_DEFINE_ENUM_UNARY_OPERATOR     (~, PROVIDE_NOT)
    ENUM_UTILS_DEFINE_ENUM_SHIFT_OPERATOR     (<<, PROVIDE_SHIFT_LEFT)
    ENUM_UTILS_DEFINE_ENUM_SHIFT_OPERATOR     (>>, PROVIDE_SHIFT_RIGHT)
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (+, PROVIDE_ADDITION)
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (-, PROVIDE_SUBTRACTION)
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (*, PROVIDE_MULTIPLICATION)
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (/, PROVIDE_DIVISION)
    ENUM_UTILS_DEFINE_ENUM_OPERATOR2          (%, PROVIDE_MODULO)
    ENUM_UTILS_DEFINE_ENUM_UNARY_OPERATOR     (+, PROVIDE_UNARY_PLUS)
    ENUM_UTILS_DEFINE_ENUM_UNARY_OPERATOR     (-, PROVIDE_UNARY_MINUS)
    ENUM_UTILS_DEFINE_ENUM_INCREMENT_OPERATOR (++, PROVIDE_INCREMENT)
    ENUM_UTILS_DEFINE_ENUM_INCREMENT_OPERATOR (--, PROVIDE_DECREMENT)
  } // namespace operators
} // namespace enum_utils

#undef ENUM_UTILS_USING_ENUM
#undef ENUM_UTILS_DEFINE_ENUM_OPERATOR2
#undef ENUM_UTILS_DEFINE_ENUM_SHIFT_OPERATOR
#undef ENUM_UTILS_DEFINE_ENUM_UNARY_OPERATOR
#undef ENUM_UTILS_DEFINE_ENUM_INCREMENT_OPERATOR
#endif
