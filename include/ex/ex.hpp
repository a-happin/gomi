#ifndef EX_HPP
#define EX_HPP

namespace ex
{
  using std::size_t;

  template <typename Result, typename ... Args>
  using function_t = auto (Args ...) -> Result;

  template <typename T, size_t N>
  using raw_array_t = T[N];
} // namespace ex
#endif
