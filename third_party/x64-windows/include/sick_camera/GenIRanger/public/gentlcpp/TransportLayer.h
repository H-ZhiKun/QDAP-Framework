// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "BasicTypes.h"

#include "Module.h"
#include "CApi.h"
#include "Interface.h"

#include <chrono>
#include <memory>
#include <vector>

namespace gentlcpp {

class CApi;
class Event;
class Interface;
class InterfaceInfo;
class Port;
class Producer;

/**
 * Represents the GenTL system module, also known as transport layer.
 *
 * Use Producer::openTransportLayer() to get a shared pointer to the instance.
 */
#ifdef SWIG
class TransportLayer : public Module
#else
class TransportLayer : public std::enable_shared_from_this<TransportLayer>,
                       public Module
#endif
{
public:
  ~TransportLayer() noexcept;

  virtual std::shared_ptr<Event> registerEvent(GenTL::EVENT_TYPE_LIST eventType)
    override;
  bool updateInterfaceList(std::chrono::milliseconds timeout);
  std::shared_ptr<Port> getPort();
  std::vector<std::shared_ptr<const InterfaceInfo>> getInterfaces();
  GenTL::TL_HANDLE getHandle();

  std::shared_ptr<Interface> openInterface(const InterfaceId& interfaceId);

private:
  friend class Producer;

  TransportLayer(std::shared_ptr<const CApi> cApi,
                 std::shared_ptr<Producer> producer);
  TransportLayer(const TransportLayer&) = delete;

  std::vector<InterfaceId> getInterfaceList() const;
  InterfaceId getInterfaceId(uint32_t index) const;

private:
  // Hold a reference to parent to make sure objects are destructed in correct
  // order
  std::shared_ptr<const Producer> mParent;
  GenTL::TL_HANDLE mHandle;
};

}
