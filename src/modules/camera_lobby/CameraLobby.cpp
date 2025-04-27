#include "CameraLobby.h"
#include "kits/camera/CameraManagerBase.h"
#include <qobject.h>

#if defined(USE_KIT_CAMERA_BAUMER)
#include "kits/camera/baumer/BaumerManager.h"
#endif

#if defined(USE_KIT_CAMERA_HIK)

#endif

namespace _Modules
{

    CameraLobby::CameraLobby(QObject *parent) : ModuleBase(parent)
    {
    }

    CameraLobby::~CameraLobby() noexcept
    {
        for (const auto &it : m_vManager)
        {
            it->stop();
        }
    }

    bool CameraLobby::start(const YAML::Node &config)
    {
        Q_UNUSED(config)
        std::string configPath;
#if defined(USE_KIT_CAMERA_BAUMER)
        m_vManager.push_back(std::make_unique<_Kits::BaumerManager>());
#endif

#if defined(USE_KIT_CAMERA_HIK)
        m_vManager.push_back(std::make_unique<WindowsInfo>());
#endif
        for (const auto &it : m_vManager)
        {
            it->start(configPath);
        }
        return true;
    }

    bool CameraLobby::stop()
    {

        return true;
    }

} // namespace _Modules
