#include "ModuleBase.h"
#include "kits/required/invoke/InvokeBase.h"
#include <QDebug>
#include <QMetaMethod>
#include <any>
#include <memory>
#include <qmetaobject.h>
#include <qobject.h>

namespace _Kits
{
    ModuleBase::ModuleBase(QObject *parent) : QObject(parent)
    {
    }
    ModuleBase::~ModuleBase()
    {
    }
    bool ModuleBase::customization(const std::unordered_map<RegisterKey, std::list<std::any>> &lvTasks)
    {
        for (const auto &[key, funcs] : lvTasks)
        {
            auto keyword = std::get<0>(key);
            auto metaFinder = m_mapMetaMethods.find(keyword);
            if (metaFinder == m_mapMetaMethods.end())
            {
                qDebug() << this->metaObject()->className() << " not find signal: " << keyword;
                continue;
            }
            QMetaMethod metaSignal = metaFinder->second;
            // 检查 metaSignal 是否是信号
            if (metaSignal.methodType() != QMetaMethod::Signal)
            {
                qDebug() << this->metaObject()->className() << " is not a signal: " << keyword;
                continue;
            }

            for (const auto &fun : funcs)
            {
                auto pkg = std::any_cast<std::shared_ptr<_Kits::InvokeBase>>(fun);

                auto metaSlot = pkg->getPrivateSlot();
                if (!metaSlot.isValid())
                {
                    qDebug() << this->metaObject()->className() << " invoke create failed: " << keyword;
                    continue;
                }
                // 连接信号和槽
                bool success = QObject::connect(this, metaSignal, pkg.get(), metaSlot);
                if (!success)
                {
                    qDebug() << this->metaObject()->className() << " connection failed: " << keyword;
                    continue;
                }
            }
        }
        return true;
    }

    void ModuleBase::tidyMetaMethod()
    {
        const QMetaObject *metaObject = this->metaObject();
        for (int i = 0; i < metaObject->methodCount(); ++i)
        {
            QMetaMethod method = metaObject->method(i);
            if (method.methodType() == QMetaMethod::Signal || method.methodType() == QMetaMethod::Slot)
            {
                m_mapMetaMethods.emplace(method.name().toStdString(), method);
            }
        }
    }

    QMetaMethod ModuleBase::getMetaMethod(const std::string &funcName)
    {
        auto finder = m_mapMetaMethods.find(funcName);
        if (finder == m_mapMetaMethods.end())
        {
            return {};
        }
        return finder->second;
    }

    bool ModuleBase::tisMetaConnect(ModuleBase *objSignal, const std::string &signalName, ModuleBase *objSlot, const std::string &slotName)
    {
        if (objSignal == nullptr || objSlot == nullptr)
        {
            return false;
        }
        auto metaSignal = objSignal->getMetaMethod(signalName);
        auto metaSlot = objSlot->getMetaMethod(slotName);
        if (!metaSignal.isValid() || !metaSlot.isValid())
        {
            return false;
        }
        bool success = QObject::connect(objSignal, metaSignal, objSlot, metaSlot);
        if (!success)
        {
            qDebug() << "tisMetaConnect failed: " << signalName << "-" << slotName;
        }
        return success;
    }
} // namespace _Kits