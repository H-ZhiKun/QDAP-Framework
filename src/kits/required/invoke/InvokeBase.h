#pragma once
#include "kits/required/log/CRossLogger.h"
#include "kits/required/thread_pool/ConcurrentPool.h"
#include "tis_global/Struct.h"
#include <QImage>
#include <QObject>
#include <qmetaobject.h>
#include <qtmetamacros.h>

namespace _Kits
{
    class InvokeBase : public QObject
    {
        Q_OBJECT

      public:
        explicit InvokeBase(const std::string &path, bool isAsync);
        virtual ~InvokeBase() noexcept;
        QMetaMethod getPrivateSlot();

      protected:
        std::string m_taskName;
        bool m_isAsync;
    };
} // namespace _Kits
