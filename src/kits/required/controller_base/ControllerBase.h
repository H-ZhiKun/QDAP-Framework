#pragma once
#include "kits/required/factory/ControllerRegister.h"
#include "kits/required/factory/FunctionWapper.h"
#include "kits/required/invoke/InvokeCreator.h"
#include "tis_global/EnumClass.h"
#include "tis_global/Function.h"
#include <functional>
#include <qlogging.h>
#include <string>
#include <tuple>
#include <utility>

namespace _Controllers
{

#define TASK_LIST_BEGIN                                                                                                                    \
    static void initRouting()                                                                                                              \
    {
#define SYNC_TASK_ADD(path, func)  instance().registerSelf(path, &func, false)
#define ASYNC_TASK_ADD(path, func) instance().registerSelf(path, &func, true)
#define TASK_LIST_END              }

    template <typename T, bool AutoCreation = true>
    class ControllerBase
    {
      public:
        static const bool isAutoCreation = AutoCreation;

        virtual ~ControllerBase()
        {
        }

      protected:
        ControllerBase()
        {
        }
        static T &instance()
        {
            static T m_ins_;
            return m_ins_;
        }

        template <typename Func>
        void registerSelf(const std::string &path, Func &&func, bool isAsync)
        {
            auto boundFunc = _Kits::FunctionWapper::wapper(func, static_cast<T *>(this));
            _Kits::ControllerRegister::registerTaskRoutes(path,
                                                          _Kits::InvokeCreator<decltype(boundFunc)>::make_invoke(path, isAsync, boundFunc));
        }

      private:
        class TaskRegistrator
        {
          public:
            TaskRegistrator()
            {
                if (AutoCreation)
                {
                    T::initRouting();
                }
            }
        };

        friend TaskRegistrator;
        static TaskRegistrator registrator_;
        virtual void *touch()
        {
            return &registrator_;
        }

      public:
        static int m_int_;
    };

    template <typename T, bool AutoCreation>
    typename ControllerBase<T, AutoCreation>::TaskRegistrator ControllerBase<T, AutoCreation>::registrator_;
} // namespace _Controllers