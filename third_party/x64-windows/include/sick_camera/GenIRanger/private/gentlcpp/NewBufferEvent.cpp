// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/NewBufferEvent.h"

#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Module.h"
#include "../../public/gentlcpp/Tracing.h"

namespace gentlcpp {

NewBufferEvent::NewBufferEvent(std::shared_ptr<const CApi> cApi,
                               std::shared_ptr<const Module> parentModule,
                               GenTL::EVENT_TYPE_LIST eventType,
                               GenTL::EVENTSRC_HANDLE eventSrcHandle)
  : Event(cApi, parentModule, eventType, eventSrcHandle)
{
  gentlcpp::trace("NewBufferEvent()");
}

NewBufferEvent::~NewBufferEvent()
{
  gentlcpp::trace("~NewBufferEvent()");
}

GenTL::S_EVENT_NEW_BUFFER
NewBufferEvent::getData(std::chrono::milliseconds timeout) const
{
  GenTL::S_EVENT_NEW_BUFFER event;
  Event::getData(&event, sizeof(event), timeout);
  return event;
}

GenTL::S_EVENT_NEW_BUFFER NewBufferEvent::getData() const
{
  GenTL::S_EVENT_NEW_BUFFER event;
  Event::getData(&event, sizeof(event));
  return event;
}

}
