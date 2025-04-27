#pragma once
#include "baumer_camera/bgapi2_genicam.hpp"
#include "kits/object_pool/ObjectPool.h"
#include "tis_global/Struct.h"
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>
#include <vector>

namespace _Kits
{
    class BaumerCamera : public QObject
    {
        Q_OBJECT
      public:
        explicit BaumerCamera(BGAPI2::Device *dev);
        virtual ~BaumerCamera();
        static void BGAPI2CALL BufferHandler(void *callBackOwner, BGAPI2::Buffer *pBufferFilled);
        bool start();
        void storeImg(BGAPI2::Buffer *);
      signals:
        void sendImage(std::shared_ptr<TIS_Info::CameraLobby::ImageBuffer> img);

      private:
        void openDataStream();
        void addBufferToStream();
        std::string m_ip;
        BGAPI2::Device *m_pDevice = nullptr;
        BGAPI2::DataStream *m_pStream = nullptr;
        std::vector<BGAPI2::Buffer *> m_vBuffers;
        bool m_bSuccess = false;
        std::unique_ptr<ObjectPool<TIS_Info::CameraLobby::ImageBuffer>> m_ImageBufferPools;
    };
} // namespace _Kits