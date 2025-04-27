#include "QmlCommunication.h"
#include <QMetaEnum>
#include <qdebug.h>
#include <qmetaobject.h>
#include <qobject.h>
#include <qvariant.h>

namespace _Modules
{
    QmlCommunication::QmlCommunication(QObject *parent) : QObject(parent)
    {
    }

    QmlCommunication::~QmlCommunication()
    {
    }
    bool QmlCommunication::start()
    {
        tidyMetaMethod();
        return true;
    }
    void QmlCommunication::behaviorFromQml(QmlActions action,
                                           const QVariant &data)
    {
        if (action == QmlActions::PageChange)
        {
            auto pageData = data.toMap();
            int pageIndex = pageData.value("pageName").toInt();
            auto pageIn = pageData.value("pageData", false).toBool();
            storePageSignal(pageIndex, pageIn);
            return;
        }

        callFromQml(static_cast<int>(action), data);
    }

    void QmlCommunication::storePageSignal(int pageIndex, bool pageIn)
    {
        if (pageIn)
        {
            auto finder = m_mapEnumSignals.find(pageIndex);
            if (finder == m_mapEnumSignals.end())
            {
                return;
            }

            m_mapPageSignal.emplace(finder->first, finder->second);
        }
        else
        {
            m_mapPageSignal.erase(pageIndex);
        }
    }

    void QmlCommunication::dataFromCpp(const QVariant &value)
    {
        auto mapData = value.toMap();
        int sigIndex = mapData.value("ForQmlSignals", -1).toInt();
        // 小于20 是页面推送枚举块，其他为自定义交互区域
        QMetaMethod method;
        if (sigIndex < 20)
        {
            auto iter = m_mapPageSignal.find(sigIndex);
            if (iter == m_mapPageSignal.end())
            {
                qDebug() << "\r\nqml page not register: " << sigIndex;
                return;
            }
            method = iter->second;
        }
        else
        {
            auto iter = m_mapEnumSignals.find(sigIndex);
            if (iter == m_mapEnumSignals.end())
            {
                qDebug() << "\r\nqml index not register: " << sigIndex;
                return;
            }
            method = iter->second;
        }
        method.invoke(this, mapData["data"]);
    }

    void QmlCommunication::tidyMetaMethod()
    {
        const QMetaObject *metaObject = this->metaObject();
        for (int i = 0; i < metaObject->methodCount(); ++i)
        {
            QMetaMethod method = metaObject->method(i);
            if (method.methodType() == QMetaMethod::Signal)
            {
                std::string sigName = method.name().toStdString();
                auto pos = sigName.find("FromCpp");
                if (pos != std::string::npos)
                {
                    std::string sigKey = sigName.substr(0, pos);
                    int enumValue = keyToValue<ForQmlSignals>(sigKey.c_str());
                    m_mapEnumSignals.emplace(enumValue, method);
                }
            }
        }
    }
} // namespace _Modules