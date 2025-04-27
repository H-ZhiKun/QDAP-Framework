#include "BaumerManager.h"
#include "BaumerCamera.h"
#include <chrono>
#include <memory>
#include <qdebug>
#include <qobject.h>
#include <qtimer.h>
#include <thread>
namespace _Kits
{
    void BGAPI2CALL BaumerManager::PnPEventHandler(void *callBackOwner, BGAPI2::Events::PnPEvent *pBuffer)
    {
        BaumerManager *manager = (BaumerManager *)callBackOwner;
        std::string snNumber = pBuffer->GetSerialNumber().get();
        if (pBuffer->GetPnPType() == BGAPI2::Events::PnPType::PNPTYPE_DEVICEREMOVED)
        {
            manager->removeCamera(snNumber);
        }
        else if (pBuffer->GetPnPType() == BGAPI2::Events::PnPType::PNPTYPE_DEVICEADDED)
        {
            manager->searchCamera(snNumber);
        }
    }

    BaumerManager::BaumerManager(QObject *parent) : CameraManagerBase(parent)
    {
    }
    BaumerManager::~BaumerManager()
    {
        if (m_thInit_.joinable())
            m_thInit_.join();
    }

    bool BaumerManager::start(const std::string &configFile)
    {
        m_thInit_ = std::thread([this] {
            while (true)
            {
                initSystem();
                if (m_bInit)
                {
                    return;
                }
                deInit();
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        });

        return true;
    }

    bool BaumerManager::stop()
    {
        deInit();
        return true;
    }

    void BaumerManager::removeCamera(const std::string &snNumber)
    {
        m_mapCamera.erase(snNumber);
    }
    void BaumerManager::searchCamera(const std::string &needNumber)
    {
        try
        {
            BGAPI2::InterfaceList *ifl = m_pSystem->GetInterfaces();
            ifl->Refresh(100);
            for (auto iter = ifl->begin(); iter != ifl->end(); iter++)
            {
                auto ifc = iter->second;
                if (!ifc->IsOpen())
                {
                    continue;
                }
                BGAPI2::DeviceList *devList = ifc->GetDevices();
                devList->Refresh(100);
                for (auto dev_iter = devList->begin(); dev_iter != devList->end(); dev_iter++)
                {
                    auto pDevice = dev_iter->second;
                    // 我不确定名称是不是Baumer，目标是为了过滤掉非Baumer的相机
                    if (pDevice->GetVendor() != "Baumer")
                    {
                        continue;
                    }
                    if (pDevice->GetAccessStatus() != "RW")
                    {
                        continue;
                    }
                    std::string snNumber = pDevice->GetSerialNumber().get();
                    if (!needNumber.empty())
                    {
                        if (snNumber != needNumber)
                        {
                            continue;
                        }
                    }
                    pDevice->Open();
                    auto pCamera = std::make_unique<BaumerCamera>(pDevice);
                    if (pCamera->start())
                    {
                        QObject::connect(pCamera.get(), &BaumerCamera::sendImage, this, &BaumerManager::sendImage);
                        m_mapCamera[snNumber] = std::move(pCamera);
                    }
                }
            }
        }
        catch (BGAPI2::Exceptions::IException &ex)
        {
            qDebug() << "Error Type:" << ex.GetType().get();
            qDebug() << "Error function: " << ex.GetFunctionName().get();
            qDebug() << "Error description: " << ex.GetErrorDescription().get();
        }
    }
    void BaumerManager::initInterface()
    {
        try
        {
            BGAPI2::InterfaceList *ifl = m_pSystem->GetInterfaces();
            ifl->Refresh(100);
            for (auto iter = ifl->begin(); iter != ifl->end(); iter++)
            {
                auto ifc = iter->second;
                if (ifc->IsOpen())
                {
                    continue;
                }

                ifc->Open();
                std::string name = ifc->GetDisplayName().get();
                std::string ip = ifc->GetNode("GevInterfaceSubnetIPAddress")->GetValue().get();
                std::string mac = ifc->GetNode("GevInterfaceMACAddress")->GetValue().get();
                std::string mask = ifc->GetNode("GevInterfaceSubnetMask")->GetValue().get();

                ifc->second->RegisterPnPEvent(BGAPI2::Events::EVENTMODE_EVENT_HANDLER);
                ifc->second->RegisterPnPEventHandler(this, &BaumerManager::PnPEventHandler);
                qDebug() << "init baumer interface founded: ";
                qDebug() << "founded ip: " << ip;
                qDebug() << "founded MAC: " << mac;
                qDebug() << "founded mask: " << mask;
                searchCamera();
                m_bInit = true;
            }
        }
        catch (BGAPI2::Exceptions::IException &ex)
        {
            qDebug() << "Error Type:" << ex.GetType().get();
            qDebug() << "Error function: " << ex.GetFunctionName().get();
            qDebug() << "Error description: " << ex.GetErrorDescription().get();
        }
    }

    void BaumerManager::initSystem()
    {
        try
        {
            BGAPI2::SystemList *system_list = BGAPI2::SystemList::GetInstance();
            system_list->Refresh();
            for (auto sys_iter = system_list->begin(); sys_iter != system_list->end(); sys_iter++)
            {
                BGAPI2::System *pSystem = sys_iter->second;
                BGAPI2::String tl_type = pSystem->GetTLType();
                if (tl_type != "GEV")
                {
                    continue;
                }
                if (pSystem->IsOpen())
                {
                    continue;
                }
                pSystem->Open();
                m_pSystem = pSystem;
                initInterface();
                qDebug() << "baumer system init successed.";
            }
            if (m_pSystem == nullptr)
            {
                qDebug() << "baumer system init failed.";
            }
        }
        catch (BGAPI2::Exceptions::IException &ex)
        {
            qDebug() << "Error Type:" << ex.GetType().get();
            qDebug() << "Error function: " << ex.GetFunctionName().get();
            qDebug() << "Error description: " << ex.GetErrorDescription().get();
        }
    }

    void BaumerManager::deInit()
    {
        try
        {
            if (m_pSystem == nullptr)
            {
                return;
            }
            if (!m_pSystem->IsOpen())
            {
                return;
            }
            BGAPI2::InterfaceList *interface_list = m_pSystem->GetInterfaces();
            interface_list->Refresh(100);
            for (auto ifc_iter = interface_list->begin(); ifc_iter != interface_list->end(); ifc_iter++)
            {
                auto pInterface = ifc_iter->second;
                if (!pInterface->IsOpen())
                {
                    continue;
                }
                pInterface->UnregisterPnPEvent();
                pInterface->Close();
            }
            m_pSystem->Close();
        }
        catch (BGAPI2::Exceptions::IException &ex)
        {

            qDebug() << "Error Type:" << ex.GetType().get();
            qDebug() << "Error function: " << ex.GetFunctionName().get();
            qDebug() << "Error description: " << ex.GetErrorDescription().get();
        }
        BGAPI2::SystemList::ReleaseInstance();
    }
} // namespace _Kits