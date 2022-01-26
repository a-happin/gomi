#ifndef CHINO_CHAR_UTILS_HPP
#define CHINO_CHAR_UTILS_HPP

namespace chino::char_utils
{
  inline constexpr auto is_blank (char32_t c) noexcept -> bool
  {
    return c == U'\t' || c == U' ' || (U'\u2000' <= c && c <= U'\u200B') || c == U'\u3000';
  }
  inline constexpr auto is_endline (char32_t c) noexcept -> bool
  {
    return c == U'\n' || c == U'\r';
  }
  inline constexpr auto is_whitespace (char32_t c) noexcept -> bool
  {
    return is_blank (c) || is_endline (c);
  }
  inline constexpr auto is_digit (char32_t c) noexcept -> bool
  {
    return U'0' <= c && c <= U'9';
  }
  inline constexpr auto is_upper (char32_t c) noexcept -> bool
  {
    return U'A' <= c && c <= U'Z';
  }
  inline constexpr auto is_lower (char32_t c) noexcept -> bool
  {
    return U'a' <= c && c <= U'z';
  }
  inline constexpr auto is_alpha (char32_t c) noexcept -> bool
  {
    return is_upper (c) || is_lower (c);
  }
  inline constexpr auto is_alnum (char32_t c) noexcept -> bool
  {
    return is_digit (c) || is_alpha (c);
  }
  inline constexpr auto is_xdigit (char32_t c) noexcept -> bool
  {
    return is_digit (c) || (U'A' <= c && c <= U'F') || (U'a' <= c && c <= U'f');
  }
  inline constexpr auto is_word (char32_t c) noexcept -> bool
  {
    return is_alnum (c) || c == U'_';
  }
}
#endif

