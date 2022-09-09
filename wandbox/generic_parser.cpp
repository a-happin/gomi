#include <chino/parser.hpp>

namespace chino::parser::generic_parser
{
  namespace detail
  {
    template <typename F, typename T, typename ... Ts>
    struct foldr
    {
      using type = std::invoke_result_t <F, T, typename foldr <F, Ts ...>::type>;
    };
    template <typename F, typename T>
    struct foldr <F, T>
    {
      using type = T;
    };

    template <typename, typename, typename ...>
    struct foldl;

    template <typename F, typename T, typename T2, typename ... Ts>
    struct foldl <F, T, T2, Ts ...>
    {
      using type = typename foldl <F, std::invoke_result_t <F, T, T2>, Ts ...>::type;
    };
    template <typename F, typename T>
    struct foldl <F, T>
    {
      using type = T;
    };
  }
  template <typename F, typename T, typename ... Ts>
  using foldl_t = typename detail::foldl <F, T, Ts ...>::type;
  template <typename F, typename T, typename ... Ts>
  using foldr_t = typename detail::foldr <F, T, Ts ...>::type;

  template <typename T>
  concept Operation = requires
  {
    T::and_ ();
  };

  template <typename T, typename E, typename I, typename U, typename P, typename ... Ps>
  inline constexpr auto generic_and_impl (I & input, U && value, P && p, Ps && ... ps) noexcept -> result::result <T, E>
  {
    auto res = std::forward <P> (p) (input);
    if (is_success (res))
    {
      auto new_value = cat (std::forward <U> (value), get_success (std::move (res)));
      if constexpr (sizeof ... (Ps) == 0)
      {
        return result::success {std::move (new_value)};
      }
      else
      {
        return generic_and_impl <T, E> (input, std::move (new_value), std::move (ps) ...);
      }
    }
    else if (is_failure (res))
    {
      return result::failure <E> {get_failure (std::move (res))};
    }
    else
    {
      return {};
    }
  }

  inline constexpr auto generic_and_ = [] <auto empty_parser, auto cat> () constexpr noexcept {
    return [empty_parser = std::move (empty_parser), cat = std::move (cat)] <typename ... Ps> (Ps ... ps) constexpr noexcept
    {
      return [empty_parser = std::move (empty_parser), cat = std::move (cat), ... ps = std::move (ps)] <typename I> (I & input) constexpr noexcept
      {
        using E = make_variant <ParserResultE <Ps, I> ...>;
        auto init = empty_parser (input);
        if constexpr (sizeof ... (Ps) == 0)
        {
          return result::success {init};
        }
        else
        {
          constexpr auto impl = [&input] <typename T, typename P, typename ... Rest> (auto && self, T && value, P && p, Rest && ... rest) constexpr noexcept -> result::result <foldl_t <decltype (cat), T, ParserResultT <P, I>, ParserResultT <Rest, I> ...>, E>
          {
            auto res = std::forward <P> (p) (input);
            if (is_success (res))
            {
              auto new_value = cat (std::move (value), get_success (std::move (res)));
              if constexpr (sizeof ... (Rest) == 0)
              {
                return result::success {std::move (new_value)};
              }
              else
              {
                return self (self, input, std::move (new_value), std::forward <Rest> (rest) ...);
              }
            }
            else if (is_failure (res))
            {
              return result::failure <E> {get_failure (std::move (res))};
            }
            else
            {
              return {};
            }
          };
          return impl (impl, input, std::move (init), std::move (ps) ...);
        }
      };
    };
  };

  namespace impl
  {
    template <auto cat, typename E, typename I, typename T, typename P, typename ... Ps>
    constexpr auto and_ (I & input, T && t, P && p, Ps && ... ps) noexcept -> result::result <foldl_t <decltype (cat), ParserResultT <P, I>, ParserResultT <Ps, I> ...>, E>
    {
      auto res = std::forward <P> (p) (input);
      if (is_success (res))
      {
        auto new_res = cat (std::move (t), get_success (std::move (res)));
        if constexpr (sizeof ... (Ps) == 0)
        {
          return result::success {std::move (new_res)};
        }
        else
        {
          return and_ <cat, E> (input, std::move (new_res), std::forward <Ps> (ps) ...);
        }
      }
      else if (is_failure (res))
      {
        return result::failure <E> {get_failure (res)};
      }
      else
      {
        return {};
      }
    }

  }

  /* template <auto epsilon = [] <typename T> (T &&) constexpr noexcept { return std::tuple <> {}; }, auto cat = [] <typename ... Ts, typename T> (std::tuple <Ts ...> && lhs, T && rhs) constexpr noexcept { return std::tuple_cat (std::move (lhs), std::make_tuple (std::forward <T> (rhs))); }> */
  /* inline constexpr auto and_ = [] <typename ... Ps> (Ps ... ps) constexpr noexcept */
  /* { */
  /*   return [... ps = std::move (ps)] <typename I> (I & input) constexpr noexcept */
  /*   { */
  /*     /1* auto identity_element = epsilon (input); *1/ */
  /*     auto identity_element = std::tuple <> {}; */
  /*     return impl::and_ <cat, make_variant <ParserResultE <Ps, I> ...>> (input, std::move (identity_element), std::move (ps) ...); */
  /*   }; */
  /* }; */

  inline constexpr auto and_ = generic_and_.operator () <
    [] (auto &&) constexpr noexcept { return std::tuple <> {}; },
    [] <typename ... ts, typename t> (std::tuple <ts ...> && lhs, t && rhs) constexpr noexcept { return std::tuple_cat (std::move (lhs), std::make_tuple (std::forward <t> (rhs))); }
  > ();
    /* [] (auto &&) constexpr noexcept { return std::tuple <> {}; }, */
    /* [] <typename ... Ts, typename T> (std::tuple <Ts ...> && lhs, T && rhs) constexpr noexcept { return std::tuple_cat (std::move (lhs), std::make_tuple (std::forward <T> (rhs))); } */

  struct StringReader
  {};

  static_assert (std::is_same_v <std::invoke_result_t <decltype (and_ ([] (auto &&) { return result::success {0}; })), StringReader &>, result::result <std::tuple <int>, never>>);
}

auto main () -> int
{
}
