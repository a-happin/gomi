#ifndef ANSI_HPP
#define ANSI_HPP
#include <iostream>
#include <typed_string.hpp>

namespace ansi
{
  using std::size_t;
  using value_type = std::uint_fast8_t;

  // escape_sequence
  template <size_t N>
  struct es_t {
    value_type values [N];

    template <typename CharT>
    friend auto operator << (std::basic_ostream <CharT> & stream, const es_t & x) -> decltype (auto)
    {
      auto flags = stream.flags ();
      stream << std::dec << std::noshowbase << std::noshowpos << TYPED_STRING (CharT, "\033[");
      for (size_t index = 0; auto && elem : x.values)
      {
        if (index ++ != 0) stream << TYPED_CHAR (CharT, ';');
        stream << static_cast <int> (elem);
      }
      stream << TYPED_CHAR (CharT, 'm');
      stream.flags (std::move (flags));
      return stream;
    }
  };

  template <typename ... Ts>
  es_t (Ts && ... args) -> es_t <sizeof ... (Ts)>;

  inline namespace style
  {
    inline constexpr es_t reset       {0};
    inline constexpr es_t bold        {1};
    inline constexpr es_t faint       {2};
    inline constexpr es_t italic      {3};
    inline constexpr es_t underline   {4};
    inline constexpr es_t blink       {5};
    inline constexpr es_t blink_rapid {6};
    inline constexpr es_t reverse     {7};
    inline constexpr es_t hidden      {8};
    inline constexpr es_t crossed_out {9};
  } // namespace style

  // foreground color
  namespace fg
  {
    inline constexpr es_t black   {30};
    inline constexpr es_t red     {31};
    inline constexpr es_t green   {32};
    inline constexpr es_t yellow  {33};
    inline constexpr es_t blue    {34};
    inline constexpr es_t magenta {35};
    inline constexpr es_t cyan    {36};
    inline constexpr es_t white   {37};
    inline constexpr es_t reset   {39};

    inline constexpr auto color256 = [] (value_type x) constexpr noexcept
    {
      return es_t {38, 5, x};
    };

    inline constexpr auto rgb = [] (value_type r, value_type g, value_type b) constexpr noexcept
    {
      return es_t {38, 2, r, g, b};
    };
  } // namespace fg

  // `using` except reset
  using fg::black, fg::red, fg::green, fg::yellow, fg::blue, fg::magenta, fg::cyan, fg::white, fg::color256, fg::rgb;

  // background color
  namespace bg
  {
    inline constexpr es_t black   {40};
    inline constexpr es_t red     {41};
    inline constexpr es_t green   {42};
    inline constexpr es_t yellow  {43};
    inline constexpr es_t blue    {44};
    inline constexpr es_t magenta {45};
    inline constexpr es_t cyan    {46};
    inline constexpr es_t white   {47};
    inline constexpr es_t reset   {49};

    inline constexpr auto color256 = [] (value_type x) constexpr noexcept
    {
      return es_t {48, 5, x};
    };

    inline constexpr auto rgb = [] (value_type r, value_type g, value_type b) constexpr noexcept
    {
      return es_t {48, 2, r, g, b};
    };
  } // namespace bg
} // namespace ansi
#endif
