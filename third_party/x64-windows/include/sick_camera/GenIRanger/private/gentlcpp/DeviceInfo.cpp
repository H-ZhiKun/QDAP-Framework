// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/DeviceInfo.h"

#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Interface.h"
#include "../../public/gentlcpp/Tracing.h"

namespace gentlcpp {

DeviceInfo::~DeviceInfo()
{
  trace("~DeviceInfo(%s)", mDeviceId.c_str());
}

gentlcpp::DeviceId DeviceInfo::getId() const
{
  return mDeviceId;
}

std::string DeviceInfo::getVendor() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_VENDOR);
}

std::string DeviceInfo::getModel() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_MODEL);
}

std::string DeviceInfo::getTransportLayerType() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_TLTYPE);
}

std::string DeviceInfo::getDisplayName() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_DISPLAYNAME);
}

GenTL::DEVICE_ACCESS_STATUS_LIST DeviceInfo::getAccessStatus() const
{
  int32_t status = getInfo<int32_t>(GenTL::DEVICE_INFO_ACCESS_STATUS);
  return static_cast<GenTL::DEVICE_ACCESS_STATUS_LIST>(status);
}

std::string DeviceInfo::getUserDefinedName() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_USER_DEFINED_NAME);
}

std::string DeviceInfo::getSerialNumber() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_SERIAL_NUMBER);
}

std::string DeviceInfo::getVersion() const
{
  return getInfo<std::string>(GenTL::DEVICE_INFO_VERSION);
}

template<>
std::string DeviceInfo::getInfo(GenTL::DEVICE_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->IFGetDeviceInfo,
                   mParent->getHandle(),
                   mDeviceId.c_str(),
                   command,
                   &dataType);
}

std::shared_ptr<gentlcpp::Interface> DeviceInfo::getParent()
{
  return mParent;
}

DeviceInfo::DeviceInfo(std::shared_ptr<const CApi> cApi,
                       std::shared_ptr<Interface> iface,
                       const DeviceId& deviceId)
  : mCApi(cApi)
  , mParent(iface)
  , mDeviceId(deviceId)
{
  trace("DeviceInfo(%s)", mDeviceId.c_str());
}

}
