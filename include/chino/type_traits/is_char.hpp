#ifndef CHINO_TYPE_TRAITS_IS_CHAR_HPP
#define CHINO_TYPE_TRAITS_IS_CHAR_HPP
#include <type_traits>

namespace chino
{
  template <typename>
  struct is_char_t : std::false_type {};

  template <>
  struct is_char_t <char> : std::true_type {};
  template <>
  struct is_char_t <unsigned char> : std::true_type {};
  template <>
  struct is_char_t <signed char> : std::true_type {};
  template <>
  struct is_char_t <wchar_t> : std::true_type {};
  template <>
  struct is_char_t <char16_t> : std::true_type {};
  template <>
  struct is_char_t <char32_t> : std::true_type {};
  template <>
  struct is_char_t <char8_t> : std::true_type {};

  template <typename T>
  inline constexpr bool is_char_v = is_char_t <T>::value;

  template <typename T>
  concept is_char = is_char_v <T>;

  static_assert (is_char <char>);
  static_assert (is_char <unsigned char>);
  static_assert (is_char <signed char>);
  static_assert (is_char <wchar_t>);
  static_assert (is_char <char16_t>);
  static_assert (is_char <char32_t>);
  static_assert (is_char <char8_t>);
  static_assert (not is_char <short>);
  static_assert (not is_char <signed short>);
  static_assert (not is_char <unsigned short>);
  static_assert (not is_char <int>);
  static_assert (not is_char <signed int>);
  static_assert (not is_char <unsigned int>);
  static_assert (not is_char <long>);
  static_assert (not is_char <signed long>);
  static_assert (not is_char <unsigned long>);
  static_assert (not is_char <long long>);
  static_assert (not is_char <signed long long>);
  static_assert (not is_char <unsigned long long>);
  static_assert (not is_char <float>);
  static_assert (not is_char <double>);
  static_assert (not is_char <long double>);
}
#endif
