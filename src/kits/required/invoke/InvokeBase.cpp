#include "InvokeBase.h"

namespace _Kits
{
    InvokeBase::InvokeBase(const std::string &path, bool isAsync) : m_taskName(path), m_isAsync(isAsync)
    {
    }
    InvokeBase::~InvokeBase() noexcept
    {
    }

    QMetaMethod InvokeBase::getPrivateSlot()
    {
        const QMetaObject *metaObject = this->metaObject();
        for (int i = 0; i < metaObject->methodCount(); ++i)
        {
            QMetaMethod method = metaObject->method(i);
            if (method.methodType() == QMetaMethod::Slot && method.name() == "invoke")
            {
                return method;
            }
        }
        return QMetaMethod();
    }
} // namespace _Kits