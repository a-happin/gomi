#ifndef CHINO_TYPE_TRAITS_IS_CHAR_TYPE_HPP
#define CHINO_TYPE_TRAITS_IS_CHAR_TYPE_HPP
#include <type_traits>

namespace chino
{
  template <typename>
  struct is_char_type_t : std::false_type {};

  template <>
  struct is_char_type_t <char> : std::true_type {};
  template <>
  struct is_char_type_t <unsigned char> : std::true_type {};
  template <>
  struct is_char_type_t <signed char> : std::true_type {};
  template <>
  struct is_char_type_t <wchar_t> : std::true_type {};
  template <>
  struct is_char_type_t <char16_t> : std::true_type {};
  template <>
  struct is_char_type_t <char32_t> : std::true_type {};
  template <>
  struct is_char_type_t <char8_t> : std::true_type {};

  template <typename T>
  inline constexpr bool is_char_type_v = is_char_type_t <T> {};

  template <typename T>
  concept is_char_type = is_char_type_v <T>;

  static_assert (is_char_type <char>);
  static_assert (is_char_type <unsigned char>);
  static_assert (is_char_type <signed char>);
  static_assert (is_char_type <wchar_t>);
  static_assert (is_char_type <char16_t>);
  static_assert (is_char_type <char32_t>);
  static_assert (is_char_type <char8_t>);
  static_assert (not is_char_type <short>);
  static_assert (not is_char_type <signed short>);
  static_assert (not is_char_type <unsigned short>);
  static_assert (not is_char_type <int>);
  static_assert (not is_char_type <signed int>);
  static_assert (not is_char_type <unsigned int>);
  static_assert (not is_char_type <long>);
  static_assert (not is_char_type <signed long>);
  static_assert (not is_char_type <unsigned long>);
  static_assert (not is_char_type <long long>);
  static_assert (not is_char_type <signed long long>);
  static_assert (not is_char_type <unsigned long long>);
  static_assert (not is_char_type <float>);
  static_assert (not is_char_type <double>);
  static_assert (not is_char_type <long double>);
}
#endif
