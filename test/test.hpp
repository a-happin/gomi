#ifndef TEST_HPP
#define TEST_HPP
#include <iostream>
#include <experimental/source_location>
#include <chino/ansi.hpp>

namespace test
{
  using std::cerr;
  using std::experimental::source_location;

  static std::size_t passed = 0;
  static std::size_t total = 0;

  template <typename T>
  concept printable = requires (std::ostream & stream, const T & x)
  {
    {stream << x} -> std::same_as <std::ostream &>;
  };

  inline auto operator << (std::ostream & stream, const source_location & location) -> decltype (auto)
  {
    return stream << location.file_name () << ':' << location.line () << ':' << location.column () << ":`" << location.function_name () << "`";
  }

  template <typename T, typename U>
  requires std::equality_comparable <T>
  inline auto assert_eq (const T & lhs, const U & rhs, const source_location location = source_location::current ()) noexcept
  {
    static_assert (std::is_same_v <T, U>);
    ++ total;
    if (lhs == rhs)
    {
      ++ passed;
    }
    else
    {
      cerr << chino::ansi::red << location << ": Assertion Failed» ";
      if constexpr (printable <T>)
      {
        cerr << "\n  lhs = " << lhs << "\n  rhs = " << rhs;
      }
      else
      {
        cerr << "unprintable object";
      }
      cerr << chino::ansi::reset << std::endl;
    }
  }

  inline auto unreachable (const source_location location = source_location::current ())
  {
    ++ total;
    cerr
      << chino::ansi::red << location << ": Assertion Failed» reached to unreachable code."
      << chino::ansi::reset << std::endl;
  }

  template <typename E, typename F>
  inline auto expect_exception (F && f, const source_location location = source_location::current ())
  {
    ++ total;
    try
    {
      std::forward <F> (f) ();
    }
    catch (const E &)
    {
      ++ passed;
      return;
    }
    catch (...)
    {
      cerr
        << chino::ansi::red << location << ": Assertion Failed» expected exception, but another exception throwed."
        << chino::ansi::reset << std::endl;
      return;
    }
    cerr
      << chino::ansi::red << location << ": Assertion Failed» expected exception, but no exception throwed."
      << chino::ansi::reset << std::endl;
  }

  [[nodiscard]] inline auto result () -> int
  {
    if (passed == total)
    {
      cerr << chino::ansi::bold << chino::ansi::green << "OK. ";
    }
    else
    {
      cerr << chino::ansi::bold << chino::ansi::red << "FAILED. ";
    }
    cerr
      << chino::ansi::green << passed << " passed; "
      << (total == passed ? chino::ansi::green : chino::ansi::red) << (total - passed) << " failed; "
      << chino::ansi::reset << "(" << (total == 0 ? 1 : static_cast <double> (passed) / static_cast <double> (total)) * 100 << "%)"
      << std::endl;

    return passed != total;
  }
}

#endif

