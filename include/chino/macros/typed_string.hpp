#ifndef TYPED_STRING
#include <tuple>
#define TYPED_CHAR(T, x) get <T> (std::tuple {x, L ## x, u ## x, U ## x, u8 ## x})
#define TYPED_STRING(T, x) get <const T *> (std::tuple {x, L ## x, u ## x, U ## x, u8 ## x})
#endif
