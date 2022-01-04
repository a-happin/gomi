#ifndef STRING_READER_HPP
#define STRING_READER_HPP
#include <cstddef>
#include <string_view>
#include <regex>
#include <optional>

#include <chino/macros/typed_string.hpp>
#include <chino/enum_utils.hpp>
#include <char_utils.hpp>
/* #define TYPED_CHAR(T,x) get <T> (std::tuple {x, L##x, u##x, U##x, u8##x}) */

namespace string_reader
{
  namespace skip_string_flag_detail
  {
    enum class skip_string_flag_t : unsigned char
    {
      DEFAULT = 0u,
      CASE_INSENSITIVE = 1u << 0,
      IS_KEYWORD = 1u << 1,
    };
    ENUM_UTILS_USING_OPERATORS;
  } // namespace string_flag_detail

  using skip_string_flag_detail::skip_string_flag_t;

  // using enum
  inline constexpr auto CASE_INSENSITIVE = skip_string_flag_t::CASE_INSENSITIVE;
  inline constexpr auto IS_KEYWORD       = skip_string_flag_t::IS_KEYWORD;


  namespace skip_if_flag_detail
  {
    enum class skip_if_flag_t : unsigned char
    {
      DEFAULT = 0u,
      WHILE = 1u,
    };
  } // skip_if_flag_detail

  using skip_if_flag_detail::skip_if_flag_t;

  // using enum
  inline constexpr auto WHILE = skip_if_flag_t::WHILE;
} // namespace string_reader

namespace enum_utils::operators
{
  template <>
  inline constexpr auto provider_v <string_reader::skip_string_flag_t> = PROVIDE_BITWISE_OPERATORS;
} // namespace enum_utils::operators

namespace string_reader
{
  using std::size_t;

  template <typename CharT>
  struct string_reader_t
  {
    using Iterator = typename std::basic_string_view <CharT>::const_iterator;
    std::basic_string_view <CharT> str;
    Iterator ite = std::ranges::begin (str);

    constexpr string_reader_t () noexcept = default;
    constexpr string_reader_t (std::basic_string_view <CharT> str_) noexcept
      : str {str_}
    {}
  private:
    constexpr string_reader_t (std::basic_string_view <CharT> str_, Iterator ite_) noexcept
      : str {str_}
      , ite {ite_}
    {}
  public:
    template <size_t N>
    constexpr string_reader_t (const CharT (& str_)[N]) noexcept
      : str {str_, N}
    {}

    auto passed_string () const
    {
      return std::basic_string_view <CharT> {std::ranges::begin (str), ite};
    }
    auto remaining_string () const
    {
      return std::basic_string_view <CharT> {ite, std::ranges::end (str)};
    }

    auto can_read () const noexcept
    {
      return ite < std::ranges::end (str);
    }

    auto eof () const noexcept
    {
      return not can_read ();
    }

    auto peek () const noexcept -> decltype (auto)
    {
      return * ite;
    }

    auto starts_with (std::basic_string_view <CharT> prefix, const skip_string_flag_t & flag = skip_string_flag_t::DEFAULT) const
    {
      if (std::ranges::equal (
        std::ranges::begin (prefix),
        std::ranges::end (prefix),
        ite,
        std::min (
          ite + prefix.size (),
          std::ranges::end (prefix)
        ),
        bittest (flag, CASE_INSENSITIVE)
          ? char_utils::equal_to_case_insensitive
          : [] (CharT c1, CharT c2) constexpr noexcept { return c1 == c2; }
      ))
      {
        if (bittest (flag, IS_KEYWORD))
        {
          auto next_ite = ite + prefix.size ();
          return not (next_ite < std::ranges::end (str) && char_utils::is_word (* next_ite));
        }
        return true;
      }
      return false;
    }

    auto skip (size_t step = 1)
    {
      ite += step;
      return true;
    }

    // f: (char) -> bool
    template <typename F>
    requires requires (F && f, CharT c)
    {
      {std::forward <F> (f) (c)} -> std::convertible_to <bool>;
    }
    auto skip (F && f, const skip_if_flag_t & flag = skip_if_flag_t::DEFAULT)
    {
      if (flag == skip_if_flag_t::WHILE)
      {
        while (can_read () && std::forward <F> (f) (peek ()))
        {
          skip ();
        }
        return true;
      }
      else
      {
        if (can_read () && std::forward <F> (f) (peek ()))
        {
          skip ();
          return true;
        }
        return false;
      }
    }

    auto skip (std::basic_string_view <CharT> prefix, const skip_string_flag_t & flag = skip_string_flag_t::DEFAULT)
    {
      if (starts_with (prefix, flag))
      {
        skip (prefix.size ());
        return true;
      }
      return false;
    }

    auto skip (const std::basic_regex <CharT> & regex)
    {
      if (std::match_results <decltype (ite)> m; regex_search (ite, std::ranges::end (str), m, regex, std::regex_constants::match_continuous))
      {
        skip (m.length ());
        return true;
      }
      return false;
    }

    template <typename ... Ts>
    requires requires (string_reader_t string_reader, Ts && ... x)
    {
      {string_reader.skip (std::forward <Ts> (x) ...)} -> std::convertible_to <bool>;
    }
    auto read (Ts && ... x) -> std::optional <std::basic_string_view <CharT>>
    {
      auto first = ite;
      if (skip (std::forward <Ts> (x) ...))
      {
        return std::basic_string_view <CharT> {first, ite};
      }
      return std::nullopt;
    }
  };

  template <typename CharT>
  string_reader_t (std::basic_string_view <CharT>) -> string_reader_t <CharT>;

  template <typename CharT>
  string_reader_t (const std::basic_string <CharT> &) -> string_reader_t <CharT>;

  template <typename CharT, size_t N>
  string_reader_t (const CharT (&)[N]) -> string_reader_t <CharT>;

} // namespace string_reader

#endif
