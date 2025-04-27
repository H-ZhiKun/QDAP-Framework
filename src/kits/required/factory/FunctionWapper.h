#pragma once
#include <functional>
#include <qlogging.h>
#include <type_traits>

namespace _Kits
{
    struct FunctionWapper
    {
        // 统一入口：可处理普通函数、成员函数（带对象）、const 成员函数（带对象）

        template <typename Func, typename Obj = void>
        static auto wapper(Func &&func, Obj *obj = nullptr)
        {
            if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>)
            {
                // 成员函数（非 const 或 const 都能匹配）
                return wrap_member_function(func, obj);
            }
            else
            {
                // 普通函数或 lambda
                return std::function<std::decay_t<Func>>(std::forward<Func>(func));
            }
        }

      private:
        // 成员函数包装器（包括 const 和 非 const）
        template <typename Ret, typename ClassType, typename... Args>
        static auto wrap_member_function(Ret (ClassType::*func)(Args...), ClassType *obj)
        {
            return std::function<Ret(Args...)>([=](Args... args) -> Ret { return (obj->*func)(std::forward<Args>(args)...); });
        }

        template <typename Ret, typename ClassType, typename... Args>
        static auto wrap_member_function(Ret (ClassType::*func)(Args...) const, const ClassType *obj)
        {
            return std::function<Ret(Args...)>([=](Args... args) -> Ret { return (obj->*func)(std::forward<Args>(args)...); });
        }
    };
} // namespace _Kits
