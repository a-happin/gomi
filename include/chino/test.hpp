#ifndef CHINO_TEST_HPP
#define CHINO_TEST_HPP
#include <iostream>
#include <experimental/source_location>
#include <chino/to_string.hpp>
#include <chino/ansi.hpp>

namespace chino::test
{
  using source_location = std::experimental::source_location;

  struct test
  {
    std::size_t passed = 0;
    std::size_t total = 0;

    constexpr test () noexcept = default;
    constexpr test (const test &) noexcept = delete;
    constexpr test & operator = (const test &) noexcept = delete;
    constexpr test (test &&) noexcept = default;
    constexpr test & operator = (test &&) noexcept = default;

    constexpr auto as_int () const &&
    {
      return passed == total ? 0 : 1;
    }
    constexpr auto rate () const
    {
      return total == 0 ? 1.0 : static_cast <double> (passed) / static_cast <double> (total);
    }
    auto assert (bool pred, source_location location = source_location::current ())
    {
      ++ total;
      if (pred)
      {
        ++ passed;
      }
      else
      {
        print (location, "assert failed.");
      }
      return pred;
    }

    template <typename Expect, typename T>
    constexpr auto assert_type (T &&)
    {
      static_assert (std::is_same_v <T, Expect>);
      return true;
    }

    template <typename T, typename U>
    requires std::equality_comparable <T>
    auto assert_eq (const T & lhs, const U & rhs, source_location location = source_location::current ())
    {
      static_assert (std::is_same_v <T, U>);
      ++ total;
      if (lhs == rhs)
      {
        ++ passed;
        return true;
      }
      else
      {
        if constexpr (into_string <T, char>)
        {
          print (location, "lhs is ", to_string <char> (lhs), ", but rhs is ", to_string <char> (rhs), ".");
        }
        else
        {
          print (location, "assert_eq failed.");
        }
        return false;
      }
    }

    auto unreachable (source_location location = source_location::current ())
    {
      ++ total;
      print (location, "unreachable");
      return false;
    }

    friend auto operator << (std::ostream & stream, const test & t) -> decltype (auto)
    {
      stream << "test result: ";
      if (t.passed == t.total)
      {
        stream << ansi::bold << ansi::green << "OK.";
      }
      else
      {
        stream << ansi::bold << ansi::red << "FAILED.";
      }
      return stream << " " << ansi::green << t.passed << " passed; " << (t.total == t.passed ? ansi::green : ansi::red) << (t.total - t.passed) << " failed; " << ansi::reset << "(" << t.rate () * 100 << "%)";
    }

  private:
    template <typename ... Ts>
    void print (const source_location &location, Ts && ... xs)
    {
      std::cerr << location.file_name () << ':' << location.line () << ':' << location.column () << ": `" << location.function_name () << "`: " << ansi::red;
      (std::cerr << ... << std::forward <Ts> (xs)) << ansi::reset << '\n';
    }
  };

  template <typename F>
  requires requires (F && f, test & t)
  {
    {f (t)};
  }
  [[nodiscard]] constexpr inline auto add_test (F && f) -> int
  {
    test t;
    f (t);
    std::cerr << t << '\n';
    return std::move (t).as_int ();
  }
}
#endif
