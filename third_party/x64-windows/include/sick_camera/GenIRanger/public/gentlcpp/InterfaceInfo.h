// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "BasicTypes.h"
#include "TransportLayer.h"

#include <TLI/GenTL.h>

#include <memory>
#include <string>

namespace gentlcpp {

class CApi;
class TransportLayer;

/**
 * Represents information about an interface which can be retrieved without
 * being opening it.
 *
 * See \ref TransportLayer::getInterfaces to get instances.
 */
class InterfaceInfo
{
public:
  ~InterfaceInfo() noexcept;
  InterfaceId getId() const;
  std::string getDisplayName() const;
  std::string getTransportLayerType() const;

  template<typename T>
  T getInfo(GenTL::INTERFACE_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(T);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi,
                 mCApi->TLGetInterfaceInfo(mParent->getHandle(),
                                           mInterfaceId.c_str(),
                                           command,
                                           &dataType,
                                           &value,
                                           &size));
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

private:
  friend class TransportLayer;
  InterfaceInfo(std::shared_ptr<const CApi> cApi,
                std::shared_ptr<TransportLayer> transportLayer,
                InterfaceId interfaceId);

private:
  std::shared_ptr<const CApi> mCApi;
  std::shared_ptr<TransportLayer> mParent;
  InterfaceId mInterfaceId;
};

}
