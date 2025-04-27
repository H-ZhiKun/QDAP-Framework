// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Device.h"
#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/DataStream.h"
#include "../../public/gentlcpp/Event.h"
#include "../../public/gentlcpp/Interface.h"
#include "../../public/gentlcpp/Port.h"
#include "../../public/gentlcpp/Tracing.h"

namespace gentlcpp {

Device::~Device()
{
  trace("~Device()");
  mCApi->DevClose(mHandle);
}

GenTL::DEV_HANDLE Device::getHandle()
{
  return mHandle;
}

std::shared_ptr<Event> Device::registerEvent(GenTL::EVENT_TYPE_LIST eventType)
{
  return std::shared_ptr<Event>(
    new Event(mCApi, shared_from_this(), eventType, mHandle));
}

std::vector<gentlcpp::DataStreamId> Device::getDataStreamIds() const
{
  std::vector<DataStreamId> list;
  uint32_t count = getDataStreamCount();
  for (uint32_t i = 0; i < count; i++)
  {
    list.push_back(getDataStreamId(i));
  }
  return list;
}

std::shared_ptr<gentlcpp::DataStream>
Device::openDataStream(const DataStreamId& dataStreamId)
{
  return std::shared_ptr<DataStream>(
    new DataStream(mCApi, shared_from_this(), dataStreamId));
}

std::shared_ptr<Port> Device::getLocalPort()
{
  return std::shared_ptr<Port>(new Port(mCApi, shared_from_this(), mHandle));
}

std::shared_ptr<Port> Device::getRemotePort()
{
  GenTL::PORT_HANDLE remotePort = GENTL_INVALID_HANDLE;
  ThrowIfError(mCApi->DevGetPort(mHandle, &remotePort));
  return std::shared_ptr<Port>(new Port(mCApi, shared_from_this(), remotePort));
}

gentlcpp::DeviceId Device::getId() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_ID);
}

std::string Device::getVendor() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_VENDOR);
}

std::string Device::getModel() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_MODEL);
}

std::string Device::getTransportLayerType() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_TLTYPE);
}

std::string Device::getDisplayName() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_DISPLAYNAME);
}

GenTL::DEVICE_ACCESS_STATUS_LIST Device::getAccessStatus() const
{
  int32_t status = getInfo<int32_t>(GenTL::DEVICE_INFO_ACCESS_STATUS);
  return static_cast<GenTL::DEVICE_ACCESS_STATUS_LIST>(status);
}

std::string Device::getUserDefinedName() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_USER_DEFINED_NAME);
}

std::string Device::getSerialNumber() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_SERIAL_NUMBER);
}

std::string Device::getVersion() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_VERSION);
}

template<>
std::string Device::getInfo(GenTL::DEVICE_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->DevGetInfo, mHandle, command, &dataType);
}

Device::Device(std::shared_ptr<const CApi> cApi,
               std::shared_ptr<Interface> iface,
               const DeviceId& deviceId,
               GenTL::DEVICE_ACCESS_FLAGS_LIST accessFlags)
  : Module(cApi)
  , mParent(iface)
  , mHandle(GENTL_INVALID_HANDLE)
{
  trace("Device()");
  ThrowIfError(mCApi->IFOpenDevice(
    iface->getHandle(), deviceId.c_str(), accessFlags, &mHandle));
}

uint32_t Device::getDataStreamCount() const
{
  uint32_t count;
  ThrowIfError(mCApi->DevGetNumDataStreams(mHandle, &count));
  return count;
}

gentlcpp::DataStreamId Device::getDataStreamId(uint32_t index) const
{
  GENTL_GET_STRING(mCApi->DevGetDataStreamID, mHandle, index);
}

}
