// Copyright 2019-2020 SICK AG. All rights reserved.

#include "../public/SharedInterfaces.h"

#include "../public/gentlcpp/TransportLayer.h"

namespace geniranger {

std::vector<std::shared_ptr<const gentlcpp::InterfaceInfo>>
SharedInterfaces::getInterfaces(std::chrono::milliseconds timeout)
{
  mTransportLayer->updateInterfaceList(timeout);
  return mTransportLayer->getInterfaces();
}

std::shared_ptr<gentlcpp::Interface>
SharedInterfaces::ensureOpen(const gentlcpp::InterfaceId& id)
{
  // Check if the interface is already in the cache
  const auto cacheEntry = mOpenInterfaces.find(id);
  if (cacheEntry != mOpenInterfaces.end())
  {
    // Try to lock the weak_ptr into a shared_ptr. This might fail if the
    // interface was just closed
    const auto weakPtr = cacheEntry->second;
    std::shared_ptr<gentlcpp::Interface> openInterface = weakPtr.lock();
    if (openInterface != nullptr)
    {
      return openInterface;
    }
  }

  // The interface is not in the cache
  std::shared_ptr<gentlcpp::Interface> iface =
    mTransportLayer->openInterface(id);
  mOpenInterfaces[id] = iface;
  return iface;
}

std::shared_ptr<gentlcpp::TransportLayer> SharedInterfaces::getTransportLayer()
{
  return mTransportLayer;
}

}
