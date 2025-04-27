#pragma once
#include "kits/camera/CameraManagerBase.h"
#include "kits/required/module_base/ModuleBase.h"
#include <memory>
#include <qdebug.h>
#include <qtmetamacros.h>
#include <vector>

namespace _Modules
{

    class CameraLobby : public _Kits::ModuleBase

    {
        Q_OBJECT
        DECLARE_MODULE(CameraLobby)

      public:
        explicit CameraLobby(QObject *parent = nullptr);
        virtual ~CameraLobby() noexcept;
        virtual bool start(const YAML::Node &config) override;
        virtual bool stop() override;

      signals:
        void sendImage(const QVariant &);

      private:
        std::vector<std::unique_ptr<_Kits::CameraManagerBase>> m_vManager;
    };

} // namespace _Modules
