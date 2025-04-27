// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/TransportLayer.h"
#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Event.h"
#include "../../public/gentlcpp/Interface.h"
#include "../../public/gentlcpp/InterfaceInfo.h"
#include "../../public/gentlcpp/Port.h"
#include "../../public/gentlcpp/Producer.h"
#include "../../public/gentlcpp/Tracing.h"

namespace gentlcpp {

TransportLayer::~TransportLayer()
{
  trace("~TransportLayer()");
  mCApi->TLClose(mHandle);
}

GenTL::TL_HANDLE TransportLayer::getHandle()
{
  return mHandle;
}

std::shared_ptr<gentlcpp::Interface>
TransportLayer::openInterface(const InterfaceId& interfaceId)
{
  return std::shared_ptr<Interface>(
    new Interface(mCApi, shared_from_this(), interfaceId));
}

std::vector<std::shared_ptr<const InterfaceInfo>>
TransportLayer::getInterfaces()
{
  std::vector<std::shared_ptr<const InterfaceInfo>> infos;
  std::vector<InterfaceId> interfaces = getInterfaceList();
  for (auto& interfaceName : interfaces)
  {
    infos.push_back(std::shared_ptr<const InterfaceInfo>(
      new InterfaceInfo(mCApi, shared_from_this(), interfaceName)));
  }
  return infos;
}

bool TransportLayer::updateInterfaceList(std::chrono::milliseconds timeout)
{
  bool changed = false;
  ThrowIfError(
    mCApi->TLUpdateInterfaceList(mHandle, &changed, timeout.count()));
  return changed;
}

std::shared_ptr<Port> TransportLayer::getPort()
{
  return std::shared_ptr<Port>(new Port(mCApi, shared_from_this(), mHandle));
}

std::shared_ptr<Event>
TransportLayer::registerEvent(GenTL::EVENT_TYPE_LIST eventType)
{
  return std::shared_ptr<Event>(
    new Event(mCApi, shared_from_this(), eventType, mHandle));
}

TransportLayer::TransportLayer(std::shared_ptr<const CApi> cApi,
                               std::shared_ptr<Producer> producer)
  : Module(cApi)
  , mParent(producer)
  , mHandle(GENTL_INVALID_HANDLE)
{
  trace("TransportLayer()");
  ThrowIfError(mCApi->TLOpen(&mHandle));
}


std::vector<gentlcpp::InterfaceId> TransportLayer::getInterfaceList() const
{
  std::vector<InterfaceId> list;
  uint32_t num = 0;
  ThrowIfError(mCApi->TLGetNumInterfaces(mHandle, &num));
  for (uint32_t i = 0; i < num; i++)
  {
    list.push_back(getInterfaceId(i));
  }
  return list;
}

gentlcpp::InterfaceId TransportLayer::getInterfaceId(uint32_t index) const
{
  GENTL_GET_STRING(mCApi->TLGetInterfaceID, mHandle, index);
}

}
