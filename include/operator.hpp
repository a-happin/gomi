#ifndef OPERATOR_HPP
#define OPERATOR_HPP
#include <utility>
#define AUTO_RETURN(...) noexcept (noexcept ((__VA_ARGS__))) -> decltype((__VA_ARGS__)) {return (__VA_ARGS__);}

namespace gomi {
  // NOTE: operator ::

  struct post_increment_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (std::forward <T> (t) ++)
  };

  struct post_decrement_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (std::forward <T> (t) --)
  };

  // NOTE: operator ()

  struct subscript_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) [std::forward <U> (u)])
  };

  // NOTE: operator .
  // NOTE: operator ->

  struct increment_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (++ std::forward <T> (t))
  };

  struct decrement_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (-- std::forward <T> (t))
  };

  struct positive_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (+ std::forward <T> (t))
  };
  
  struct negate_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (- std::forward <T> (t))
  };
  
  struct logical_not_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (! std::forward <T> (t))
  };
  
  struct bit_not_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (~ std::forward <T> (t))
  };

  template <typename To>
  struct cast_t {
    template <typename From>
    constexpr auto operator () (From && x) const
      AUTO_RETURN (static_cast <To> (std::forward <From> (x)))
  };

  struct indirection_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (* std::forward <T> (t))
  };
  
  struct address_of_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (& std::forward <T> (t))
  };
  
  struct sizeof_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (sizeof (std::forward <T> (t)))
  };
  
  // NOTE: operator alignof
  
  struct sizeof_parameter_pack_t {
    template <typename ... Ts>
    constexpr auto operator () (Ts && ...) const
      AUTO_RETURN (sizeof ... (Ts))
  };

  // NOTE: operator typeid
  
  struct noexcept_t {
    template <typename T>
    constexpr auto operator () (T && t) const
      AUTO_RETURN (noexcept (std::forward <T> (t)))
  };

  // NOTE: operator new
  // NOTE: operator new []
  // NOTE: operator delete
  // NOTE: operator delete []

  // NOTE: operator .*
  // NOTE: operator ->*

  struct multiplies_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) * std::forward <U> (u))
  };
  
  struct divides_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) / std::forward <U> (u))
  };
  
  struct modulus_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) % std::forward <U> (u))
  };

  struct plus_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) + std::forward <U> (u))
  };
  
  struct minus_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) - std::forward <U> (u))
  };
  
  struct left_shift_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) << std::forward <U> (u))
  };
  
  struct right_shift_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) >> std::forward <U> (u))
  };
  
  struct greater_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) > std::forward <U> (u))
  };
  
  struct less_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) < std::forward <U> (u))
  };
  
  struct greater_equal_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) >= std::forward <U> (u))
  };
  
  struct less_equal_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) <= std::forward <U> (u))
  };
  
  struct equal_to_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) == std::forward <U> (u))
  };
  
  struct not_equal_to_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) != std::forward <U> (u))
  };
  
  struct bit_and_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) & std::forward <U> (u))
  };
  
  struct bit_xor_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) ^ std::forward <U> (u))
  };
  
  struct bit_or_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) | std::forward <U> (u))
  };
  
  struct logical_and_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) && std::forward <U> (u))
  };
  
  struct logical_or_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) || std::forward <U> (u))
  };
  
  struct conditional_t {
    template <typename C , typename T , typename U>
    constexpr auto operator () (C && c , T && t , U && u) const
      AUTO_RETURN (std::forward <C> (c) ? std::forward <T> (t) : std::forward <U> (u))
  };
  
  struct assign_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) = std::forward <U> (u))
  };
  
  struct assign_sum_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) += std::forward <U> (u))
  };
  using add_t = assign_sum_t;
  
  struct assign_difference_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) -= std::forward <U> (u))
  };
  using subtract_t = assign_difference_t;
  
  struct assign_product_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) *= std::forward <U> (u))
  };

  struct assign_quotient_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) /= std::forward <U> (u))
  };

  struct assign_remainder_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) %= std::forward <U> (u))
  };

  struct assign_left_shift_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) <<= std::forward <U> (u))
  };

  struct assign_right_shift_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) >>= std::forward <U> (u))
  };

  struct assign_bit_and_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) &= std::forward <U> (u))
  };

  struct assign_bit_xor_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) ^= std::forward <U> (u))
  };

  struct assign_bit_or_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) |= std::forward <U> (u))
  };

  // NOTE: operator throw

  struct comma_t {
    template <typename T , typename U>
    constexpr auto operator () (T && t , U && u) const
      AUTO_RETURN (std::forward <T> (t) , std::forward <U> (u))
  };
} // namespace gomi
#undef AUTO_RETURN
#endif // OPERATOR_HPP
