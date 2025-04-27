#pragma once
#include "BaumerCamera.h"
#include "kits/camera/CameraManagerBase.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace _Kits
{
    class BaumerManager : public CameraManagerBase
    {
        Q_OBJECT
      public:
        explicit BaumerManager(QObject *parent = nullptr);
        virtual ~BaumerManager();
        virtual bool start(const std::string &configFile) override;
        virtual bool stop() override;
        void searchCamera(const std::string &needNumber = "");
        void removeCamera(const std::string &snNumber);

      private:
        static void BGAPI2CALL PnPEventHandler(void *callBackOwner, BGAPI2::Events::PnPEvent *pBuffer);
        void initSystem();
        void initInterface();
        void deInit();
        std::thread m_thInit_;
        BGAPI2::System *m_pSystem = nullptr;
        bool m_bInit = false;

        std::unordered_map<std::string, std::unique_ptr<BaumerCamera>> m_mapCamera;
    };
} // namespace _Kits