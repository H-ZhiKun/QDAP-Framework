#pragma once
#include "DeviceInfo.h"
#include "kits/required/module_base/ModuleBase.h"
#include "tis_global/Function.h"
#include <json/json.h>
#include <memory>
#include <qobject.h>

namespace _Modules
{

    class DeviceManager : public _Kits::ModuleBase
    {
        Q_OBJECT
        DECLARE_MODULE(
            DeviceManager) // 1 完成模块类型 注册宏申明，在cpp中去完成注册。
        TIS_CONNECT(TIS_Info::DeviceManager::notifyDiskInfo,
                    TIS_Info::QmlPrivateEngine::callFromCpp)

      public:
        explicit DeviceManager(QObject *parent = nullptr);
        virtual ~DeviceManager() noexcept;
        DeviceManager(const DeviceManager &) = delete;
        DeviceManager &operator=(const DeviceManager &) = delete;
        virtual bool start(const YAML::Node &config) override;
        virtual bool stop() override;

      signals:
        void notifyDeviceInfo(const QVariant &); // 信号用于传输设备状态信息
        void notifyDiskInfo(const QVariant &);   // 用于传输磁盘容量信息

      private:
        std::unique_ptr<_Kits::DeviceInfo> m_ptrDevice{nullptr};
    };

} // namespace _Modules
