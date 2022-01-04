#ifndef CHINO_TYPE_TRAITS_IS_CLASS_TEMPLATE_OF
#define CHINO_TYPE_TRAITS_IS_CLASS_TEMPLATE_OF
#include <type_traits>

namespace chino
{
  namespace detail
  {
    template <typename, template <typename ...> class>
    struct is_class_template_of_t : std::false_type {};

    template <typename ... Args, template <typename ...> class C>
    struct is_class_template_of_t <C <Args ...>, C>: std::true_type {};
  }
  template <typename T, template <typename ...> class C>
  struct [[deprecated("std::arrayに対応できないので個別でtraitsを作るべき。")]] is_class_template_of_t : detail::is_class_template_of_t <std::remove_cvref_t <T>, C> {};

  template <typename T, template <typename ...> class C>
  [[deprecated("std::arrayに対応できないので個別でtraitsを作るべき。")]]
  inline constexpr bool is_class_template_of_v = is_class_template_of_t <T, C> {};

  template <typename T, template <typename ...> class C>
  concept class_template_of = is_class_template_of_v <T, C>;
}

#endif
