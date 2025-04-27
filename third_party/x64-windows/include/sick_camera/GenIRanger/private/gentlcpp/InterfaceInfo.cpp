// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/InterfaceInfo.h"

#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Tracing.h"
#include "../../public/gentlcpp/TransportLayer.h"

namespace gentlcpp {

InterfaceInfo::~InterfaceInfo()
{
  trace("~InterfaceInfo(%s)", mInterfaceId.c_str());
}

gentlcpp::InterfaceId InterfaceInfo::getId() const
{
  return mInterfaceId;
}

std::string InterfaceInfo::getDisplayName() const
{
  return getInfo<std::string>(GenTL::INTERFACE_INFO_DISPLAYNAME);
}

std::string InterfaceInfo::getTransportLayerType() const
{
  return getInfo<std::string>(GenTL::INTERFACE_INFO_TLTYPE);
}

template<>
std::string InterfaceInfo::getInfo(GenTL::INTERFACE_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->TLGetInterfaceInfo,
                   mParent->getHandle(),
                   mInterfaceId.c_str(),
                   command,
                   &dataType);
}

InterfaceInfo::InterfaceInfo(std::shared_ptr<const CApi> cApi,
                             std::shared_ptr<TransportLayer> transportLayer,
                             InterfaceId interfaceId)
  : mCApi(cApi)
  , mParent(transportLayer)
  , mInterfaceId(interfaceId)
{
  trace("InterfaceInfo(%s)", mInterfaceId.c_str());
}

}
