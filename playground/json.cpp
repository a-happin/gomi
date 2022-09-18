#include <chino/overload.hpp>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iostream>
#define FOR_IMPL(i,b,e) for (auto [i, i ## _end] = std::tuple {(b), (e)}; i < i ## _end; ++ i)
#define FOR(...) FOR_IMPL(__VA_ARGS__)
#define rep(i,n) FOR(i,0zu,n)

#if defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpadded"
#endif

struct JSON : std::variant <std::monostate, bool, double, std::string, std::vector <JSON>, std::unordered_map <std::string, JSON>>
{
  using variant::variant;

  friend auto operator << (std::ostream & stream, const JSON & json) -> decltype (auto)
  {
    return json.print (stream, 2);
  }

  auto stringify (std::size_t spacer = 0) const
  {
    std::ostringstream ss;
    print (ss, spacer);
    return std::move (ss).str ();
  }

private:
  auto print (std::ostream & stream, const std::size_t & spacer) const -> std::ostream &
  {
    std::size_t indent = 0;
    auto impl = [&stream, &spacer, &indent] (auto & self, const JSON & json) -> void
    {
      std::visit (chino::overload {
        [&stream] (const std::monostate &) { stream << "null"; },
        [&stream] (const bool & b) { stream << (b ? "true" : "false"); },
        [&stream] (const double & val) { stream << val; },
        [&stream] (const std::string & str) { stream << '"' << str << '"'; },
        [&self, &stream, &spacer, &indent] (const std::vector <JSON> & arr) noexcept {
          stream << '[';
          indent += spacer;
          for (bool is_first = true; auto && elem : arr)
          {
            if (is_first)
            {
              is_first = false;
            }
            else
            {
              stream << ',';
              if (spacer > 0)
              {
                stream << '\n';
                rep (i, indent)
                {
                  stream << ' ';
                }
              }
            }
            self (self, elem);
          }
          indent -= spacer;
          stream << ']';
        },
        [&self, &stream, &spacer, &indent] (const std::unordered_map <std::string, JSON> & object) noexcept {
          stream << '{';
          indent += spacer;
          for (bool is_first = true; auto && [key, value] : object)
          {
            if (is_first)
            {
              is_first = false;
            }
            else
            {
              stream << ',';
              if (spacer > 0)
              {
                stream << '\n';
                rep (i, indent)
                {
                  stream << ' ';
                }
              }
            }
            stream << '"' << key << '"';
            if (spacer > 0)
            {
              stream << ' ';
            }
            self (self, value);
          }
          indent -= spacer;
          stream << '}';
        },
      }, json);
    };
    impl (impl, * this);
    return stream;
  }
};

#if defined (__GNUC__)
  #pragma GCC diagnostic pop
#endif

auto main () -> int
{
  JSON a;
  std::cout << a << "\n";
}
