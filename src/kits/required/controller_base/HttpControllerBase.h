#pragma once
#include "kits/required/factory/ControllerRegister.h"
#include "kits/required/factory/FunctionWapper.h"
#include "tis_global/EnumClass.h"
#include "tis_global/Field.h"
#include "tis_global/Function.h"
#include <QHttpServerRequest>
#include <QHttpServerResponse>

namespace _Controllers
{

#define HTTP_LIST_BEGIN                                                                                                                    \
    static void initRouting()                                                                                                              \
    {
#define HTTP_ADD(httpRoute, func) instance().registerSelf(httpRoute, &func)
#define HTTP_LIST_END             }

    template <typename T, bool AutoCreation = true>
    class HttpControllerBase
    {
      public:
        static const bool isAutoCreation = AutoCreation;

        virtual ~HttpControllerBase()
        {
        }

      protected:
        HttpControllerBase()
        {
        }
        static T &instance()
        {
            static T m_ins_;
            return m_ins_;
        }

        template <typename Func>
        void registerSelf(const std::string &httpRoute, Func &&func)
        {
            auto boundFunc = _Kits::FunctionWapper::wapper(func, static_cast<T *>(this));
            _Kits::ControllerRegister::registerHttpPath(httpRoute, boundFunc);
        }

      private:
        class Registrator
        {
          public:
            Registrator()
            {
                if (AutoCreation)
                {
                    T::initRouting();
                }
            }
        };

        friend Registrator;
        static Registrator registrator_;

        virtual void *touch()
        {
            return &registrator_;
        }
    };

    template <typename T, bool AutoCreation>
    typename HttpControllerBase<T, AutoCreation>::Registrator HttpControllerBase<T, AutoCreation>::registrator_;

} // namespace _Controllers