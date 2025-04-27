// Copyright 2019-2020 SICK AG. All rights reserved.

#pragma once

#include "Avoid.h"

#include "../public/gentlcpp/BasicTypes.h"

#include <chrono>
#include <map>
#include <memory>
#include <vector>

namespace gentlcpp {
class InterfaceInfo;
class Interface;
class TransportLayer;
}

namespace geniranger {

/**
 * Keeps track of the open interface for a transport layer instance. This
 * prevents errors from GenTL by avoiding opening the same interface more than
 * once.
 */
class SharedInterfaces
{
public:
  explicit SharedInterfaces(
    std::shared_ptr<gentlcpp::TransportLayer> transportLayer)
    : mTransportLayer(transportLayer)
  {
  }

  /** \return information on all available interfaces. */
  std::vector<std::shared_ptr<const gentlcpp::InterfaceInfo>>
  getInterfaces(std::chrono::milliseconds timeout);

  /**
   * Opens an interface unless it is not already opened.
   *
   * \return the open interface
   */
  std::shared_ptr<gentlcpp::Interface>
  ensureOpen(const gentlcpp::InterfaceId& id);

  /**
   * \return the underlying \ref gentlcpp::TransportLayer
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID std::shared_ptr<gentlcpp::TransportLayer> getTransportLayer();

private:
  SharedInterfaces(const SharedInterfaces&) = delete;
  SharedInterfaces& operator=(const SharedInterfaces&) = delete;

private:
  std::shared_ptr<gentlcpp::TransportLayer> mTransportLayer;

  /**
   * A cache of opened interfaces. Uses weak_ptr so that this cache prevents the
   * interfaces from closing when no longer used.
   */
  std::map<gentlcpp::InterfaceId, std::weak_ptr<gentlcpp::Interface>>
    mOpenInterfaces;
};

}
