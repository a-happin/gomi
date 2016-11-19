#ifndef VARIANT_HPP
#define VARIANT_HPP
#include <utility>
#include <type_traits>
#include <stdexcept>
VARIANT_HPP
struct in_place_left_t {};
struct in_place_right_t {};

template <typename T , typename U , bool = std::is_trivially_destructible <T>::value && std::is_trivially_destructible <U>::value , bool = std::is_trivially_copyable <T>::value && std::is_trivially_copyable <U>::value>
struct variant_base {
  union {
    bool is_right;
    char padding1 [alignof (T)];
    char padding2 [alignof (U)];
  };
  union {
    T t;
    U u;
  };

  template <typename ... Args>
  constexpr variant_base (in_place_left_t , Args && ... args) noexcept
    : is_right {false}
    , t {std::forward <Args> (args) ...}
  {}

  template <typename ... Args>
  constexpr variant_base (in_place_right_t , Args && ... args) noexcept
    : is_right {true}
    , u {std::forward <Args> (args) ...}
  {}

  variant_base (const variant_base & other) noexcept {
    if (other.is_right) {
      new (& u) U (other.u);
    }
    else {
      new (& t) T (other.t);
    }
  }

  variant_base (variant_base && other) noexcept {
    if (other.is_right) {
      new (& u) U (std::move (other.u));
    }
    else {
      new (& t) T (std::move (other.t));
    }
  }

  auto operator = (const variant_base & other) noexcept {
    if (is_right) {
      u.~ U ();
    }
    else {
      t.~ T ();
    }
    if (other.is_right) {
      new (& u) U (other.u);
    }
    else {
      new (& t) T (other.t);
    }
  }

  auto operator = (variant_base && other) noexcept {
    if (is_right) {
      u.~ U ();
    }
    else {
      t.~ T ();
    }
    if (other.is_right) {
      new (& u) U (std::move (other.u));
    }
    else {
      new (& t) T (std::move (other.t));
    }
  }

  ~ variant_base () noexcept {
    if (is_right) {
      u.~ U ();
    }
    else {
      t.~ T ();
    }
  }
};

template <typename T , typename U>
struct variant_base <T , U , true , false> {
  union {
    bool is_right;
    char padding1 [alignof (T)];
    char padding2 [alignof (U)];
  };
  union {
    T t;
    U u;
  };

  template <typename ... Args>
  constexpr variant_base (in_place_left_t , Args && ... args) noexcept
    : is_right {false}
    , t {std::forward <Args> (args) ...}
  {}

  template <typename ... Args>
  constexpr variant_base (in_place_right_t , Args && ... args) noexcept
    : is_right {true}
    , u {std::forward <Args> (args) ...}
  {}

  variant_base (const variant_base & other) noexcept {
    if (other.is_right) {
      new (& u) U (other.u);
    }
    else {
      new (& t) T (other.t);
    }
  }

  variant_base (variant_base && other) noexcept {
    if (other.is_right) {
      new (& u) U (std::move (other.u));
    }
    else {
      new (& t) T (std::move (other.t));
    }
  }

  auto operator = (const variant_base & other) noexcept {
    if (is_right) {
      u.~ U ();
    }
    else {
      t.~ T ();
    }
    if (other.is_right) {
      new (& u) U (other.u);
    }
    else {
      new (& t) T (other.t);
    }
  }

  auto operator = (variant_base && other) noexcept {
    if (is_right) {
      u.~ U ();
    }
    else {
      t.~ T ();
    }
    if (other.is_right) {
      new (& u) U (std::move (other.u));
    }
    else {
      new (& t) T (std::move (other.t));
    }
  }
};

template <typename T , typename U>
struct variant_base <T , U , true , true> {
  union {
    bool is_right;
    char padding1 [alignof (T)];
    char padding2 [alignof (U)];
  };
  union {
    T t;
    U u;
  };

  template <typename ... Args>
  constexpr variant_base (in_place_left_t , Args && ... args) noexcept
    : is_right {false}
    , t {std::forward <Args> (args) ...}
  {}

  template <typename ... Args>
  constexpr variant_base (in_place_right_t , Args && ... args) noexcept
    : is_right {true}
    , u {std::forward <Args> (args) ...}
  {}
};

template <typename T , typename U>
struct variant : private variant_base <T , U> {
  using variant_base <T , U>::variant_base;

  constexpr variant (T && v) noexcept
    : variant_base <T , U> (in_place_left_t {} , std::move (v))
  {}

  constexpr variant (U && v) noexcept
    : variant_base <T , U> (in_place_right_t {} , std::move (v))
  {}

  constexpr auto is_right () const {
    return variant_base <T , U>::is_right;
  }

  constexpr auto left () const noexcept -> decltype (auto) {
    if (is_right ()) {
      throw std::runtime_error {"bad variant access"};
    }
    return (variant_base <T , U>::t);
  }

  constexpr auto left () noexcept -> decltype (auto) {
    if (is_right ()) {
      throw std::runtime_error {"bad variant access"};
    }
    return (variant_base <T , U>::t);
  }

  constexpr auto right () const noexcept -> decltype (auto) {
    if (is_right ()) {
      return (variant_base <T , U>::u);
    }
    throw std::runtime_error {"bad variant access"};
  }

  constexpr auto right () noexcept -> decltype (auto) {
    if (is_right ()) {
      return (variant_base <T , U>::u);
    }
    throw std::runtime_error {"bad variant access"};
  }
};
#endif // VARIANT_HPP
