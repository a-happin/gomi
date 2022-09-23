#include <type_traits>
#include <concepts>
#include <utility>
#include <numeric>
#include <cstdint>
#include <chino/never.hpp>
#include <chino/type_traits/at.hpp>

template <std::size_t>
struct in_place_index_t
{
};

template <std::size_t, std::size_t, typename ...>
struct binary_union;

template <std::size_t first, std::size_t last, typename ... Ts> requires (std::is_trivially_destructible_v <Ts> && ...)
struct binary_union <first, last, Ts ...>
{
  inline static constexpr auto mid = std::midpoint (first, last);
  inline static constexpr auto is_left_leaf  = first + 1 == mid;
  inline static constexpr auto is_right_leaf = mid + 1 == last;

  using type = binary_union;
  using Left  = typename std::conditional_t <is_left_leaf,  chino::at <first, Ts ...>, binary_union <first, mid, Ts ...>>::type;
  using Right = typename std::conditional_t <is_right_leaf, chino::at <mid, Ts ...>,   binary_union <mid,  last, Ts ...>>::type;

  union
  {
    [[no_unique_address]] Left left;
    [[no_unique_address]] Right right;
  };

  constexpr binary_union () noexcept
    /* requires (std::is_trivially_default_constructible_v <Left> && std::is_trivially_default_constructible_v <Right>) */
  = default;

  constexpr binary_union (const binary_union &) noexcept
    /* requires (std::is_trivially_copy_constructible_v <Left> && std::is_trivially_copy_constructible_v <Right>) */
  = default;

  constexpr binary_union (binary_union &&) noexcept
    /* requires (std::is_trivially_move_constructible_v <Left> && std::is_trivially_move_constructible_v <Right>) */
  = default;

  constexpr auto operator = (const binary_union &) noexcept -> binary_union &
    /* requires (std::is_trivially_copy_assignable_v <Left> && std::is_trivially_copy_assignable_v <Right>) */
  = default;

  constexpr auto operator = (binary_union &&) noexcept -> binary_union &
    /* requires (std::is_trivially_move_assignable_v <Left> && std::is_trivially_move_assignable_v <Right>) */
  = default;

  constexpr ~ binary_union () noexcept
    /* requires (std::is_trivially_destructible_v <Left> && std::is_trivially_destructible_v <Right>) */
  = default;

  /* constexpr ~ binary_union () noexcept */
  /*   requires (std::is_destructible_v <Left> && std::is_destructible_v <Right> && (not (std::is_trivially_destructible_v <Left> && std::is_trivially_destructible_v <Right>))) */
  /* {} */

  template <typename ... Args>
  constexpr binary_union (in_place_index_t <first>, Args && ... args) requires (is_left_leaf)
    : left {std::forward <Args> (args) ...}
  {}

  template <std::size_t i, typename ... Args> requires (first <= i && i < mid)
  constexpr binary_union (in_place_index_t <i> in_place_index, Args && ... args) requires (not is_left_leaf)
    : left {in_place_index, std::forward <Args> (args) ...}
  {}

  template <typename ... Args>
  constexpr binary_union (in_place_index_t <mid>, Args && ... args) requires (is_right_leaf)
    : right {std::forward <Args> (args) ...}
  {}

  template <std::size_t i, typename ... Args> requires (mid <= i && i < last)
  constexpr binary_union (in_place_index_t <i> in_place_index, Args && ... args) requires (not is_right_leaf)
    : right {in_place_index, std::forward <Args> (args) ...}
  {}
};

template <std::size_t first, std::size_t last, typename ... Ts> requires (not (std::is_trivially_destructible_v <Ts> && ...))
struct binary_union <first, last, Ts ...>
{
  inline static constexpr auto mid = std::midpoint (first, last);
  inline static constexpr auto is_left_leaf  = first + 1 == mid;
  inline static constexpr auto is_right_leaf = mid + 1 == last;

  using type = binary_union;
  using Left  = typename std::conditional_t <is_left_leaf,  chino::at <first, Ts ...>, binary_union <first, mid, Ts ...>>::type;
  using Right = typename std::conditional_t <is_right_leaf, chino::at <mid, Ts ...>,   binary_union <mid,  last, Ts ...>>::type;

  union
  {
    [[no_unique_address]] Left left;
    [[no_unique_address]] Right right;
  };

  constexpr binary_union () noexcept
    /* requires (std::is_trivially_default_constructible_v <Left> && std::is_trivially_default_constructible_v <Right>) */
  = default;

  constexpr binary_union (const binary_union &) noexcept
    /* requires (std::is_trivially_copy_constructible_v <Left> && std::is_trivially_copy_constructible_v <Right>) */
  = default;

  constexpr binary_union (binary_union &&) noexcept
    /* requires (std::is_trivially_move_constructible_v <Left> && std::is_trivially_move_constructible_v <Right>) */
  = default;

  constexpr auto operator = (const binary_union &) noexcept -> binary_union &
    /* requires (std::is_trivially_copy_assignable_v <Left> && std::is_trivially_copy_assignable_v <Right>) */
  = default;

  constexpr auto operator = (binary_union &&) noexcept -> binary_union &
    /* requires (std::is_trivially_move_assignable_v <Left> && std::is_trivially_move_assignable_v <Right>) */
  = default;

  constexpr ~ binary_union () noexcept
    /* requires (std::is_trivially_destructible_v <Left> && std::is_trivially_destructible_v <Right>) */
  {}

  /* constexpr ~ binary_union () noexcept */
  /*   requires (std::is_destructible_v <Left> && std::is_destructible_v <Right> && (not (std::is_trivially_destructible_v <Left> && std::is_trivially_destructible_v <Right>))) */
  /* {} */

  template <typename ... Args>
  constexpr binary_union (in_place_index_t <first>, Args && ... args) requires (is_left_leaf)
    : left {std::forward <Args> (args) ...}
  {}

  template <std::size_t i, typename ... Args> requires (first <= i && i < mid)
  constexpr binary_union (in_place_index_t <i> in_place_index, Args && ... args) requires (not is_left_leaf)
    : left {in_place_index, std::forward <Args> (args) ...}
  {}

  template <typename ... Args>
  constexpr binary_union (in_place_index_t <mid>, Args && ... args) requires (is_right_leaf)
    : right {std::forward <Args> (args) ...}
  {}

  template <std::size_t i, typename ... Args> requires (mid <= i && i < last)
  constexpr binary_union (in_place_index_t <i> in_place_index, Args && ... args) requires (not is_right_leaf)
    : right {in_place_index, std::forward <Args> (args) ...}
  {}
};


template <typename ... Ts>
using binary_union_t = binary_union <0, sizeof ... (Ts), Ts ...>;

template <typename ... Ts>
struct variant
{
  using storage_t = binary_union_t <Ts ...>;

  [[no_unique_address]] storage_t storage;
  union
  {
    std::uint_fast8_t current;
    char dummy_[alignof (storage_t)];
  };

  constexpr variant ()
    noexcept (std::is_nothrow_default_constructible_v <chino::at_t <0, Ts ...>>)
    requires (std::is_default_constructible_v <chino::at_t <0, Ts ...>>)
    : storage {in_place_index_t <0> {}}
    , current {0}
  {}

  /* constexpr ~ variant () */
  /*   noexcept (std::is_nothrow_destructible_v <storage_t>) */
  /*   /1* requires std::is_trivially_destructible_v <storage_t> *1/ */
  /* = default; */

  /* constexpr ~ variant () */
  /*   noexcept (std::is_nothrow_destructible_v <storage_t>) */
  /*   requires std::is_destructible_v <storage_t> && (not std::is_trivially_destructible_v <storage_t>) */
  /* {} */
};

struct empty_class
{
  friend constexpr auto operator <=> (empty_class, empty_class) noexcept = default;
};

struct non_trivial
{
  constexpr non_trivial () noexcept {}
  constexpr non_trivial (const non_trivial &) noexcept {}
  constexpr non_trivial (non_trivial &&) noexcept {}
  constexpr ~non_trivial () noexcept {}
  friend constexpr auto operator <=> (non_trivial, non_trivial) noexcept = default;
};

using test = variant <int, empty_class, char>;
static_assert (alignof (binary_union_t <int, empty_class, non_trivial>) == 4);
static_assert (!std::is_trivially_default_constructible_v <test>);
static_assert (std::is_trivially_copy_constructible_v <test>);
static_assert (std::is_trivially_move_constructible_v <test>);
static_assert (std::is_trivially_copy_assignable_v <test>);
static_assert (std::is_trivially_move_assignable_v <test>);
static_assert (std::is_trivially_destructible_v <test>);

/* static_assert (std::is_default_constructible_v <variant <int, non_trivial> {}>); */
/* static_assert (std::is_copy_constructible_v <test>); */
/* static_assert (std::is_move_constructible_v <test>); */
/* static_assert (std::is_copy_assignable_v <test>); */
/* static_assert (std::is_move_assignable_v <test>); */
/* static_assert (std::is_destructible_v <test>); */
/* static_assert (std::is_same_v <make_binary_union_t <int, double, float>, int>); */

auto main () -> int
{
  variant <int, non_trivial> a;
  constexpr binary_union_t <int, non_trivial> b {in_place_index_t <0> {}};
  static_assert (std::is_destructible_v <int> && std::is_destructible_v <non_trivial> && (not (std::is_trivially_destructible_v <int> && std::is_trivially_destructible_v <non_trivial>)));
}
