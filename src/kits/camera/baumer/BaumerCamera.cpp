#include "BaumerCamera.h"
#include "kits/object_pool/ObjectPool.h"
#include <QDebug>
#include <memory>
#include <qlogging.h>
#include <qobject.h>
#include <qtmetamacros.h>

namespace _Kits
{
    BaumerCamera::BaumerCamera(BGAPI2::Device *dev) : m_pDevice(dev), QObject(nullptr)
    {
    }
    BaumerCamera::~BaumerCamera()
    {
        try
        {
            if (m_pDevice != NULL)
            {
                if (m_pDevice->GetRemoteNodeList()->GetNodePresent("AcquisitionAbort"))
                {
                    m_pDevice->GetRemoteNode("AcquisitionAbort")->Execute();
                }
                m_pDevice->GetRemoteNode("AcquisitionStop")->Execute();
            }
        }
        catch (BGAPI2::Exceptions::IException &ex)
        {
            qDebug() << "Error Type:" << ex.GetType().get();
            qDebug() << "Error function: " << ex.GetFunctionName().get();
            qDebug() << "Error description: " << ex.GetErrorDescription().get();
        }

        // 停止相机的采集流
        try
        {
            if (m_pStream != NULL)
            {
                m_pStream->StopAcquisition();
                m_pStream->GetBufferList()->DiscardAllBuffers();
            }
        }
        catch (BGAPI2::Exceptions::IException &ex)
        {
            qDebug() << "Error Type:" << ex.GetType().get();
            qDebug() << "Error function: " << ex.GetFunctionName().get();
            qDebug() << "Error description: " << ex.GetErrorDescription().get();
        }
        // 释放相机的采集流
        try
        {
            if (m_pStream != NULL)
            {
                while (m_pStream->GetBufferList()->size() > 0)
                {
                    auto *buffer = m_pStream->GetBufferList()->begin()->second;
                    m_pStream->GetBufferList()->RevokeBuffer(buffer);
                    delete buffer;
                }
                // 关闭相机的数据流
                m_pStream->Close();
                // 关闭相机的设备
                m_pDevice->Close();
            }
        }
        catch (BGAPI2::Exceptions::IException &ex)
        {
            qDebug() << "Error Type:" << ex.GetType().get();
            qDebug() << "Error function: " << ex.GetFunctionName().get();
            qDebug() << "Error description: " << ex.GetErrorDescription().get();
        }
    }

    void BGAPI2CALL BaumerCamera::BufferHandler(void *callBackOwner, BGAPI2::Buffer *pBufferFilled)
    {
        BaumerCamera *pCamera = static_cast<BaumerCamera *>(callBackOwner);
        if (pBufferFilled == NULL)
        {
            return;
        }
        if (pBufferFilled->GetIsIncomplete())
        {
            pBufferFilled->QueueBuffer();
        }
        else
        {
            pCamera->storeImg(pBufferFilled);
            pBufferFilled->QueueBuffer();
        }
    }
    void BaumerCamera::storeImg(BGAPI2::Buffer *pBufferFilled)
    {
        uint64_t width = pBufferFilled->GetWidth();
        uint64_t height = pBufferFilled->GetHeight();
        unsigned char *imageData = static_cast<unsigned char *>(pBufferFilled->GetMemPtr()) + pBufferFilled->GetImageOffset();
        auto imgBuffer = m_ImageBufferPools->getObject(width, height);
        // 直接使用 img->data，不需要每次 resize
        if (imgBuffer->data.size() == width * height)
        {
            memcpy(imgBuffer->data.data(), imageData, imgBuffer->data.size());
            imgBuffer->name = m_ip;
            imgBuffer->height = height;
            imgBuffer->width = width;
            imgBuffer->timestamp = pBufferFilled->GetTimestamp();
            imgBuffer->pixFormat = pBufferFilled->GetPixelFormat().get();
            emit sendImage(imgBuffer);
        }
        else
        {
            qDebug() << "image data size error.";
        }
    }
    bool BaumerCamera::start()
    {
        openDataStream();
        if (m_bSuccess)
        {
            m_ip = m_pDevice->GetRemoteNode("GevCurrentIPAddress")->GetValue().get();
            m_ImageBufferPools = std::make_unique<ObjectPool<TIS_Info::CameraLobby::ImageBuffer>>();
        }
        return m_bSuccess;
    }

    void BaumerCamera::openDataStream()
    {
        auto pStreams = m_pDevice->GetDataStreams();
        pStreams->Refresh();
        if (pStreams->size() > 0)
        {
            m_pStream = (*pStreams)[0];
            if (!m_pStream->IsOpen())
            {
                m_pStream->Open();
                addBufferToStream();
                m_pStream->RegisterNewBufferEvent(BGAPI2::Events::EVENTMODE_EVENT_HANDLER);
                m_pStream->RegisterNewBufferEventHandler(this, &BaumerCamera::BufferHandler);
                m_pStream->StartAcquisitionContinuous();
                m_bSuccess = true;
            }
        }
    }
    void BaumerCamera::addBufferToStream()
    {
        if (!m_pStream->IsOpen())
        {
            auto pBufferList = m_pStream->GetBufferList();
            for (int i = 0; i < 10; i++)
            {
                auto buffer = new BGAPI2::Buffer;
                pBufferList->Add(buffer);
                m_vBuffers.push_back(buffer);
            }
            // 不知道baumer的demo为什么要先申请，再调用QueueBuffer, 照抄的
            for (auto iter = pBufferList->begin(); iter != pBufferList->end(); ++iter)
            {
                iter->second->QueueBuffer();
            }
        }
    }
} // namespace _Kits