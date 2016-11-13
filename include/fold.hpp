#ifndef FOLD_HPP
#define FOLD_HPP
#include <at.hpp>
#include <utility>

namespace gomi {
  namespace detail {
    template <typename , typename ...>
    struct foldl_impl;

    template <size_t ... Indices , typename ... Us>
    struct foldl_impl <index_sequence <Indices ...> , Us ...> {
      template <typename F , typename T , typename ... Rest>
      static constexpr auto eval (F & f , T && c , typename at <Indices , Us && ...>::type ... half , Rest && ... rest)
      {
        return foldl_impl <make_index_sequence <sizeof ... (Rest) / 2> , Rest ...>::eval (
          f ,
          foldl_impl <make_index_sequence <sizeof ... (Indices) / 2> , typename at <Indices , Us ...>::type ...>::eval (
            f ,
            std::forward <T> (c) ,
            std::forward <typename at <Indices , Us ...>::type> (half) ...
          ) ,
          std::forward <Rest> (rest) ...
        );
      }

      template <typename F , typename T , typename U>
      static constexpr auto eval (F & f , T && c , U && x)
        -> decltype (f (std::forward <T> (c) , std::forward <U> (x)))
      {
        return f (std::forward <T> (c) , std::forward <U> (x));
      }

      template <typename F , typename T>
      static constexpr auto eval (F & , T && c)
        -> decltype (std::forward <T> (c))
      {
        return std::forward <T> (c);
      }
    };


    template <typename , typename ...>
    struct foldr_impl;

    template <size_t ... Indices , typename ... Us>
    struct foldr_impl <index_sequence <Indices ...> , Us ...> {
      template <typename F , typename T , typename ... Rest>
      static constexpr auto eval (F & f , T && c , typename at <Indices , Us && ...>::type ... half , Rest && ... rest) {
        return foldr_impl <make_index_sequence <sizeof ... (Indices) / 2> , typename at <Indices , Us ...>::type ...>::eval (
          f ,
          foldr_impl <make_index_sequence <sizeof ... (Rest) / 2> , Rest ...>::eval (
            f ,
            std::forward <T> (c) ,
            std::forward <Rest> (rest) ...
          ) ,
          std::forward <typename at <Indices , Us ...>::type> (half) ...
        );
      }

      template <typename F , typename T , typename U>
      static constexpr auto eval (F & f , T && c , U && x)
        -> decltype (f (std::forward <U> (x) , std::forward <T> (c)))
      {
        return f (std::forward <U> (x) , std::forward <T> (c));
      }

      template <typename F , typename T>
      static constexpr auto eval (F & , T && c)
        -> decltype (std::forward <T> (c))
      {
        return std::forward <T> (c);
      }
    };
  } // namespace detail

  template <typename F , typename T , typename ... Us>
  constexpr auto foldl (F f , T && c , Us && ... xs) {
    return detail::foldl_impl <make_index_sequence <sizeof ... (Us) / 2> , Us ...>::eval (
      f ,
      std::forward <T> (c) ,
      std::forward <Us> (xs) ...
    );
  }

  template <typename F , typename T , typename ... Us>
  constexpr auto foldr (F f , T && c , Us && ... xs) {
    return detail::foldr_impl <make_index_sequence <sizeof ... (Us) / 2> , Us ...>::eval (
      f ,
      std::forward <T> (c) ,
      std::forward <Us> (xs) ...
    );
  }
} // namespace gomi
#endif // FOLD_HPP
