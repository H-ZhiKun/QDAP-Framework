// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Interface.h"

#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Device.h"
#include "../../public/gentlcpp/DeviceInfo.h"
#include "../../public/gentlcpp/Event.h"
#include "../../public/gentlcpp/Port.h"
#include "../../public/gentlcpp/Tracing.h"
#include "../../public/gentlcpp/TransportLayer.h"

namespace gentlcpp {

Interface::~Interface()
{
  trace("~Interface()");
  mCApi->IFClose(mHandle);
}

GenTL::IF_HANDLE Interface::getHandle()
{
  return mHandle;
}

std::shared_ptr<Event>
Interface::registerEvent(GenTL::EVENT_TYPE_LIST eventType)
{
  return std::shared_ptr<Event>(
    new Event(mCApi, shared_from_this(), eventType, mHandle));
}

bool Interface::updateDeviceList(std::chrono::milliseconds timeout)
{
  bool hasChanged = false;
  ThrowIfError(
    mCApi->IFUpdateDeviceList(mHandle, &hasChanged, timeout.count()));
  return hasChanged;
}

std::vector<std::shared_ptr<DeviceInfo>> Interface::getDevices()
{
  std::vector<std::shared_ptr<DeviceInfo>> infos;

  uint32_t deviceCount = getDeviceCount();
  for (uint32_t index = 0; index < deviceCount; ++index)
  {
    infos.push_back(std::shared_ptr<DeviceInfo>(
      new DeviceInfo(mCApi, shared_from_this(), getDeviceId(index))));
  }
  return infos;
}


std::shared_ptr<gentlcpp::Device>
Interface::openDevice(const DeviceId& deviceId,
                      GenTL::DEVICE_ACCESS_FLAGS_LIST accessFlags)
{
  return std::shared_ptr<Device>(
    new Device(mCApi, shared_from_this(), deviceId, accessFlags));
}

gentlcpp::DeviceId Interface::getId() const
{
  return getInfo<std::string>(GenTL::INTERFACE_INFO_ID);
}

std::string Interface::getDisplayName() const
{
  return getInfo<std::string>(GenTL::INTERFACE_INFO_DISPLAYNAME);
}

std::string Interface::getTransportLayerType() const
{
  return getInfo<std::string>(GenTL::INTERFACE_INFO_TLTYPE);
}

std::shared_ptr<Port> Interface::getPort()
{
  return std::shared_ptr<Port>(new Port(mCApi, shared_from_this(), mHandle));
}

template<>
std::string Interface::getInfo(GenTL::INTERFACE_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->IFGetInfo, mHandle, command, &dataType);
}

Interface::Interface(std::shared_ptr<const CApi> cApi,
                     std::shared_ptr<TransportLayer> transportLayer,
                     const InterfaceId& interfaceId)
  : Module(cApi)
  , mParent(transportLayer)
  , mHandle(GENTL_INVALID_HANDLE)
{
  trace("Interface()");

  ThrowIfError(mCApi->TLOpenInterface(
    transportLayer->getHandle(), interfaceId.c_str(), &mHandle));
}

uint32_t Interface::getDeviceCount()
{
  uint32_t count = 0;
  ThrowIfError(mCApi->IFGetNumDevices(mHandle, &count));
  return count;
}

gentlcpp::DeviceId Interface::getDeviceId(uint32_t index)
{
  GENTL_GET_STRING(mCApi->IFGetDeviceID, mHandle, index);
}

}
