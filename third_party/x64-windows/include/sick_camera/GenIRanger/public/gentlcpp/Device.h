// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "BasicTypes.h"
#include "Module.h"

#include <memory>
#include <string>
#include <vector>

namespace gentlcpp {

class Interface;
class DataStream;
class CApi;
class Port;

/**
 * Represents a device in the GenTL the device module.
 *
 * Use \ref Interface::openDevice to get an instance.
 */
#ifdef SWIG
class Device : public Module
#else
class Device : public std::enable_shared_from_this<Device>, public Module
#endif
{
public:
    //~Device() noexcept;
    ~Device() noexcept;

  GenTL::DEV_HANDLE getHandle();

  std::shared_ptr<Event> registerEvent(GenTL::EVENT_TYPE_LIST eventType);

  std::vector<DataStreamId> getDataStreamIds() const;
  std::shared_ptr<DataStream> openDataStream(const DataStreamId& dataStreamId);

  std::shared_ptr<Port> getLocalPort();
  std::shared_ptr<Port> getRemotePort();
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
    ThrowIfError(mCApi->DevGetInfo(mHandle, command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::DEVICE_INFO_CMD_LIST customCommand) const;


  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(static_cast<GenTL::DEVICE_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

private:
  friend class Interface;

  Device(std::shared_ptr<const CApi> cApi,
         std::shared_ptr<Interface> iface,
         const DeviceId& deviceId,
         GenTL::DEVICE_ACCESS_FLAGS_LIST accessFlags);
  Device(const Device&) = delete;

  uint32_t getDataStreamCount() const;
  DataStreamId getDataStreamId(uint32_t index) const;

private:
  // Hold a reference to parent to make sure objects are destructed in correct
  // order
  std::shared_ptr<const Interface> mParent;
  GenTL::DEV_HANDLE mHandle;
};

}
