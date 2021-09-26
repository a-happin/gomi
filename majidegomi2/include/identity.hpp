#ifndef IDENTITY_HPP
#define IDENTITY_HPP

namespace gomi {
  template <typename T>
  struct Identity {
    using type = T;
  };
} // namespace gomi
#endif // IDENTITY_HPP
