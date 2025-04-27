#include "DeviceManager.h"
#include "kits/required/thread_pool/ConcurrentPool.h"
#include "tis_global/EnumClass.h"
#include "tis_global/Field.h"
#include <json/json.h>
#include <memory>

#ifdef _WIN32
#include "WindowsInfo.h"
#endif

namespace _Modules
{
    using namespace _Kits;

    // 2 完成模块类型
    // 注册宏实现，如果将来要制作动态库时避免发生错误。
    DeviceManager::DeviceManager(QObject *parent) : ModuleBase(parent)
    {
    }

    DeviceManager::~DeviceManager() noexcept
    {
    }

    bool DeviceManager::start(const YAML::Node &config)
    {
        Q_UNUSED(config)
#if defined(_WIN32)
        m_ptrDevice = std::make_unique<WindowsInfo>();
#elif defined(USE_LINUX_INFO)
        m_ptrDevice = std::make_unique<LinuxInfo>();
#else
#endif
        if (m_ptrDevice != nullptr && m_ptrDevice->start())
        {
            threadPool().runEvery(
                "DeviceManager::generate",
                [this] {
                    auto jsData = m_ptrDevice->logicalDriveInfo();
                    QVariantMap mapData;
                    mapData[TIS_Info::QmlCommunication::strForQmlSignals] = QVariant::fromValue(TIS_Info::QmlCommunication::ForQmlSignals::main_page);
                    mapData[TIS_Info::QmlCommunication::strData] = QVariant();
                    emit notifyDiskInfo(mapData);
                },
                5000);
            // threadPool().runEvery(
            //     "DeviceManager::deviceInfo",
            //     [this] { m_ptrDevice->deviceInfo(); },
            //     1000);
        }
        return true;
    }

    bool DeviceManager::stop()
    {

        return true;
    }

} // namespace _Modules
