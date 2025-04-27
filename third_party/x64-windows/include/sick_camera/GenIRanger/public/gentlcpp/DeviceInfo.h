// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "BasicTypes.h"
#include "CApi.h"
#include "Interface.h"

#include <TLI/GenTL.h>

#include <memory>
#include <string>

namespace gentlcpp 
{

class CApi;
class Interface;

/**
 * Represents information about a device which can be retrieved without being
 * connected to it.
 *
 * See \ref Interface::getDevices to get instances.
 */
class DeviceInfo
{
public:
  ~DeviceInfo() noexcept;

  DeviceId getId() const;
  std::string getVendor() const;
  std::string getModel() const;
  std::string getTransportLayerType() const;
  std::string getDisplayName() const;
  GenTL::DEVICE_ACCESS_STATUS_LIST getAccessStatus() const;
  std::string getUserDefinedName() const;
  std::string getSerialNumber() const;
  std::string getVersion() const;

  template<typename T>
  T getInfo(GenTL::DEVICE_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(T);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi,
                 mCApi->IFGetDeviceInfo(mParent->getHandle(),
                                        getId().c_str(),
                                        command,
                                        &dataType,
                                        &value,
                                        &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::DEVICE_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(static_cast<GenTL::DEVICE_INFO_CMD_LIST>(customCommand));
  }

  std::shared_ptr<gentlcpp::Interface> getParent();

#ifdef SWIG
  EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

private:
  friend class Interface;

  DeviceInfo(std::shared_ptr<const CApi> cApi,
             std::shared_ptr<gentlcpp::Interface> iface,
             const DeviceId& deviceId);
  DeviceInfo(const DeviceInfo&) = delete;

private:
  std::shared_ptr<const CApi> mCApi;
  std::shared_ptr<gentlcpp::Interface> mParent;
  DeviceId mDeviceId;
};

}
