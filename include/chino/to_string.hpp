#ifndef CHINO_TO_STRING_HPP
#define CHINO_TO_STRING_HPP
#include <sstream>
#include <array>
#include <span>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <valarray>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <optional>
#include <variant>
#include <chino/macros/typed_string.hpp>
#include <chino/type_traits/is_char_type.hpp>

namespace chino
{
  namespace detail
  {
    template <typename, typename>
    struct to_string_impl;
  }

  template <typename T, typename CharT>
  concept into_string = requires (const T & x)
  {
    {detail::to_string_impl <CharT, T>::impl (x)} -> std::same_as <std::basic_string <CharT>>;
  };

  namespace detail
  {
    template <typename CharT>
    struct to_string_cpo
    {
      template <into_string <CharT> T>
      auto operator () (const T & x) const -> std::basic_string <CharT>
      {
        return to_string_impl <CharT, T>::impl (x);
      }
    };
  }

  template <typename CharT>
  inline constexpr detail::to_string_cpo <CharT> to_string;

  namespace detail {
    template <typename CharT>
    struct to_string_impl <CharT, bool>
    {
      static auto impl (const bool & x) -> std::basic_string <CharT>
      {
        return x ? TYPED_STRING (CharT, "true") : TYPED_STRING (CharT, "false");
      }
    };

    template <typename CharT>
    struct to_string_impl <CharT, std::nullptr_t>
    {
      static auto impl (const std::nullptr_t &) -> std::basic_string <CharT>
      {
        return TYPED_STRING (CharT, "nullptr");
      }
    };

    template <typename CharT>
    struct to_string_impl <CharT, std::nullopt_t>
    {
      static auto impl (const std::nullopt_t &) -> std::basic_string <CharT>
      {
        return TYPED_STRING (CharT, "nullopt");
      }
    };

    template <typename CharT>
    struct to_string_impl <CharT, std::monostate>
    {
      static auto impl (const std::monostate &) -> std::basic_string <CharT>
      {
        return TYPED_STRING (CharT, "monostate");
      }
    };

    // char
    template <typename CharT, is_char_type T>
    requires (sizeof (T) <= sizeof (CharT))
    struct to_string_impl <CharT, T>
    {
      static auto impl (const T & x) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_CHAR (CharT, '\'');
        if (sizeof (T) == sizeof (char))
        {
          stream << static_cast <CharT> (static_cast <unsigned char> (x));
        }
        else
        {
          stream << static_cast <CharT> (x);
        }
        stream << TYPED_CHAR (CharT, '\'');
        return std::move (stream).str ();
      }
    };

    // char (oversized) = delete
    template <typename CharT, is_char_type T>
    requires (sizeof (T) > sizeof (CharT))
    struct to_string_impl <CharT, T>
    {
    };

    // integral, floating_point
    template <typename CharT, typename T>
    requires (std::is_arithmetic_v <T> && !std::same_as <T, bool> && !is_char_type <T>)
    struct to_string_impl <CharT, T>
    {
      static auto impl (const T & x) -> std::basic_string <CharT>
      {
        if constexpr (std::same_as <CharT, char>)
        {
          return std::to_string (x);
        }
        else if constexpr (std::same_as <CharT, wchar_t>)
        {
          return std::to_wstring (x);
        }
        else
        {
          return (std::basic_ostringstream <CharT> {} << x).str ();
        }
      }
    };


    // byte
    template <typename CharT>
    struct to_string_impl <CharT, std::byte>
    {
      static auto impl (const std::byte & x) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_STRING (CharT, "0x") << std::hex << static_cast <unsigned int> (x);
        return std::move (stream).str ();
      }
    };


    // string
    template <typename CharT>
    struct to_string_impl <CharT, std::basic_string_view <CharT>>
    {
      static auto impl (std::basic_string_view <CharT> str) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_CHAR (CharT, '"') << str << TYPED_CHAR (CharT, '"');
        return std::move (stream).str ();
      }
    };

    template <typename CharT>
    struct to_string_impl <CharT, std::basic_string <CharT>> : to_string_impl <CharT, std::basic_string_view <CharT>> {};

    template <typename CharT>
    struct to_string_impl <CharT, const CharT *> : to_string_impl <CharT, std::basic_string_view <CharT>> {};


    // array
    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, T[]>
    {
      template <typename Arr>
      requires requires (const Arr & arr)
      {
        std::ranges::begin (arr);
        std::ranges::end (arr);
      }
      static auto impl (const Arr & arr) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_STRING (CharT, "[");
        for (std::size_t idx = 0; auto && elem : arr)
        {
          if (idx ++ != 0)
          {
            stream << TYPED_STRING (CharT, ", ");
          }
          stream << to_string <CharT> (elem);
        }
        stream << TYPED_STRING (CharT, "]");
        return std::move (stream).str ();
      }

      // charT[]は文字列
      template <std::size_t N>
      static auto impl (const CharT (& str)[N]) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_CHAR (CharT, '"');
        for (auto && c : str)
        {
          stream << c;
        }
        stream << TYPED_CHAR (CharT, '"');
        return std::move (stream).str ();
      }
    };

    template <typename CharT, into_string <CharT> T, std::size_t N>
    struct to_string_impl <CharT, T[N]> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T, std::size_t N>
    struct to_string_impl <CharT, std::array <T, N>> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T, std::size_t N>
    struct to_string_impl <CharT, std::span <T, N>> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::initializer_list <T>> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::vector <T>> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::deque <T>> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::forward_list <T>> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::list <T>> : to_string_impl <CharT, T[]> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::valarray <T>> : to_string_impl <CharT, T[]> {};

    // set
    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::set <T>>
    {
      template <typename Arr>
      requires requires (const Arr & arr)
      {
        std::ranges::begin (arr);
        std::ranges::end (arr);
      }
      static auto impl (const Arr & arr) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_STRING (CharT, "{");
        for (std::size_t idx = 0; auto && elem : arr)
        {
          if (idx ++ != 0)
          {
            stream << TYPED_STRING (CharT, ", ");
          }
          stream << to_string <CharT> (elem);
        }
        stream << TYPED_STRING (CharT, "}");
        return std::move (stream).str ();
      }
    };

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::multiset <T>> : to_string_impl <CharT, std::set <T>> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::unordered_set <T>> : to_string_impl <CharT, std::set <T>> {};

    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::unordered_multiset <T>> : to_string_impl <CharT, std::set <T>> {};


    // map
    template <typename CharT, into_string <CharT> K, into_string <CharT> T>
    struct to_string_impl <CharT, std::map <K, T>>
    {
      template <typename Arr>
      requires requires (const Arr & arr)
      {
        std::ranges::begin (arr);
        std::ranges::end (arr);
      }
      static auto impl (const Arr & arr) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_STRING (CharT, "{");
        for (std::size_t idx = 0; auto && [key, value] : arr)
        {
          if (idx ++ != 0)
          {
            stream << TYPED_STRING (CharT, ", ");
          }
          stream << to_string <CharT> (key) << TYPED_STRING (CharT, ": ") << to_string <CharT> (value);
        }
        stream << TYPED_STRING (CharT, "}");
        return std::move (stream).str ();
      }
    };

    template <typename CharT, into_string <CharT> K, into_string <CharT> T>
    struct to_string_impl <CharT, std::multimap <K, T>> : to_string_impl <CharT, std::map <K, T>> {};

    template <typename CharT, into_string <CharT> K, into_string <CharT> T>
    struct to_string_impl <CharT, std::unordered_map <K, T>> : to_string_impl <CharT, std::map <K, T>> {};

    template <typename CharT, into_string <CharT> K, into_string <CharT> T>
    struct to_string_impl <CharT, std::unordered_multimap <K, T>> : to_string_impl <CharT, std::map <K, T>> {};


    // tuple
    template <typename CharT, into_string <CharT> ... Args>
    struct to_string_impl <CharT, std::tuple <Args ...>>
    {
      template <typename Tuple>
      static auto impl (const Tuple & t) -> std::basic_string <CharT>
      {
        std::basic_ostringstream <CharT> stream;
        stream << TYPED_STRING (CharT, "(");
        std::apply ([&stream] (auto && ... args) constexpr {
          auto f = [idx = std::size_t {0}, &stream] (auto && elem) mutable
          {
            if (idx ++ != 0) stream << TYPED_STRING (CharT, ", ");
            stream << to_string <CharT> (std::forward <decltype (elem)> (elem));
          };
          (f (std::forward <decltype (args)> (args)), ...);
        }, t);
        stream << TYPED_STRING (CharT, ")");
        return std::move (stream).str ();
      }
    };

    template <typename CharT, into_string <CharT> T, into_string <CharT> U>
    struct to_string_impl <CharT, std::pair <T, U>> : to_string_impl <CharT, std::tuple <T, U>> {};


    // optional
    template <typename CharT, into_string <CharT> T>
    struct to_string_impl <CharT, std::optional <T>>
    {
      static auto impl (const std::optional <T> & opt) -> std::basic_string <CharT>
      {
        if (opt)
        {
          std::basic_ostringstream <CharT> stream;
          stream << TYPED_STRING (CharT, "optional (") << to_string <CharT> (* opt) << TYPED_STRING (CharT, ")");
          return std::move (stream).str ();
        }
        return to_string <CharT> (std::nullopt);
      }
    };


    // variant
    template <typename CharT, into_string <CharT> ... Args>
    struct to_string_impl <CharT, std::variant <Args ...>>
    {
      static auto impl (const std::variant <Args ...> & v) -> std::basic_string <CharT>
      {
        return std::visit (to_string <CharT>, v);
      }
    };
  }
}
#endif
