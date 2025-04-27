// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "BasicTypes.h"

#include "CApi.h"
#include "Module.h"
#include "Device.h"
#include "Event.h"
#include "Port.h"
#include "TransportLayer.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace gentlcpp {

class CApi;
class Device;
class DeviceInfo;
class Event;
class Port;
class TransportLayer;

/**
 * Represents an interface in the GenTL interface module.
 *
 * Use \ref TransportLayer::openInterface to get an instance.
 */
#ifdef SWIG
class Interface : public Module
#else
class Interface : public std::enable_shared_from_this<Interface>, public Module
#endif
{
public:
  ~Interface() noexcept;

  GenTL::IF_HANDLE getHandle();

  virtual std::shared_ptr<Event> registerEvent(GenTL::EVENT_TYPE_LIST eventType)
    override;

  bool updateDeviceList(std::chrono::milliseconds timeout);
  std::vector<std::shared_ptr<DeviceInfo>> getDevices();

  std::shared_ptr<Device> openDevice(
    const DeviceId& deviceId, GenTL::DEVICE_ACCESS_FLAGS_LIST accessFlags);

  DeviceId getId() const;
  std::string getDisplayName() const;
  std::string getTransportLayerType() const;
  std::shared_ptr<Port> getPort();

  // General get info methods (for the interface)
  template<typename T>
  T getInfo(GenTL::INTERFACE_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(T);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi->IFGetInfo(mHandle, command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::INTERFACE_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(
      static_cast<GenTL::INTERFACE_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

private:
  friend class TransportLayer;

  Interface(std::shared_ptr<const CApi> cApi,
            std::shared_ptr<TransportLayer> tranportLayer,
            const InterfaceId& interfaceId);
  Interface(const Interface&) = delete;

  uint32_t getDeviceCount();
  DeviceId getDeviceId(uint32_t index);

private:
  // Hold a reference to parent to make sure objects are destructed in correct
  // order
  std::shared_ptr<const TransportLayer> mParent;
  GenTL::IF_HANDLE mHandle;
};

}
