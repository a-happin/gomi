#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP
#include <utility>
#include <type_traits>

namespace gomi {
  struct dummy_t {};
  struct in_place_t {};
  template <typename> struct in_place_type_t {};
  
  template <typename T , bool = std::is_trivially_destructible <T>::value>
  union storage_t;
  
  template <typename T>
  union storage_t <T , true> {
    dummy_t dummy;
    T value;
    
    constexpr storage_t () noexcept
      : dummy {}
    {}
    
    template <typename ... Ts>
    constexpr storage_t (in_place_t , Ts && ... args) noexcept
      : value {std::forward <Ts> (args) ...}
    {}
  };
  
  template <typename T>
  union storage_t <T , false> {
    dummy_t dummy;
    T value;
    
    constexpr storage_t () noexcept
      : dummy {}
    {}
    
    template <typename ... Ts>
    constexpr storage_t (in_place_t , Ts && ... args) noexcept
      : value {std::forward <Ts> (args) ...}
    {}
    
    ~ storage_t () {}
  };
  
  template <typename T , bool = std::is_trivially_destructible <T>::value>
  struct optional_base;
  
  template <typename T>
  struct optional_base <T , true> {
    union {
      bool initialized;
      char padding [alignof (T)]; // suppress warning
    };
    storage_t <T> storage;
    
    constexpr optional_base () noexcept
      : initialized {false}
      , storage {}
    {}
    
    template <typename ... Ts>
    constexpr optional_base (in_place_t in_place, Ts && ... args) noexcept
      : initialized {true}
      , storage {in_place , std::forward <Ts> (args) ...}
    {}
    
    constexpr auto clear () noexcept {
      initialized = false;
    }
  };
  
  template <typename T>
  struct optional_base <T , false> {
    union {
      bool initialized;
      char padding [alignof (T)]; // suppress warning
    };
    storage_t <T> storage;
    
    constexpr optional_base () noexcept
      : initialized {false}
      , storage {}
    {}
    
    template <typename ... Ts>
    constexpr optional_base (in_place_t in_place, Ts && ... args) noexcept
      : initialized {true}
      , storage {in_place , std::forward <Ts> (args) ...}
    {}
    
    ~ optional_base () noexcept {
      clear ();
    }
    
    constexpr auto clear () noexcept {
      if (initialized) storage.value.~ T ();
      initialized = false;
    }
  };
  
  template <typename T>
  struct optional : private optional_base <T> {
    using value_type = T;
    using optional_base <T>::optional_base;
    using optional_base <T>::clear;
    
    constexpr optional (T && value) noexcept
      : optional_base <T> (in_place_t {} , std::move (value))
    {}
    
    constexpr optional (const T & value) noexcept
      : optional_base <T> (in_place_t {} , value)
    {}
    
    constexpr operator bool () const noexcept {
      return optional_base <T>::initialized;
    }
    
    constexpr auto operator * () const & noexcept -> decltype (auto) {
      return (optional_base <T>::storage.value);
    }
    
    constexpr auto operator -> () const & noexcept {
      return & optional_base <T>::storage.value;
    }
  };
} // namespace gomi

#endif // OPTIONAL_HPP
