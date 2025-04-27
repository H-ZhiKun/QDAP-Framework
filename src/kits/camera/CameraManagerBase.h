#pragma once
#include "tis_global/Struct.h"
#include <memory>
#include <qimage.h>
#include <qobject.h>
#include <qtmetamacros.h>

namespace _Kits
{
    class CameraManagerBase : public QObject
    {
        Q_OBJECT
      public:
        explicit CameraManagerBase(QObject *parent = nullptr);
        virtual ~CameraManagerBase();
        virtual bool start(const std::string &configFile) = 0;
        virtual bool stop() = 0;
      signals:
        void sendImage(std::shared_ptr<TIS_Info::CameraLobby::ImageBuffer> img);
    };
} // namespace _Kits