#pragma once
#include "kits/required/factory/ControllerRegister.h"
#include "kits/required/factory/FunctionWapper.h"
#include "tis_global/EnumClass.h"
#include "tis_global/Function.h"


namespace _Controllers
{

#define QML_LIST_BEGIN                                                                                                                     \
    static void initRouting()                                                                                                              \
    {
#define QML_ADD(qmlPath, func) instance().registerSelf(qmlPath, &func)
#define QML_LIST_END           }

    template <typename T, bool AutoCreation = true>
    class QmlControllerBase
    {
      public:
        static const bool isAutoCreation = AutoCreation;

        virtual ~QmlControllerBase()
        {
        }

      protected:
        QmlControllerBase()
        {
        }
        static T &instance()
        {
            static T m_ins_;
            return m_ins_;
        }

        template <typename Func>
        void registerSelf(TIS_Info::QmlCommunication::QmlActions qmlPath, Func &&func)
        {
            auto boundFunc = _Kits::FunctionWapper::wapper(func, static_cast<T *>(this));
            _Kits::ControllerRegister::registerQmlPath(static_cast<int>(qmlPath), std::move(boundFunc));
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
    typename QmlControllerBase<T, AutoCreation>::Registrator QmlControllerBase<T, AutoCreation>::registrator_;

} // namespace _Controllers