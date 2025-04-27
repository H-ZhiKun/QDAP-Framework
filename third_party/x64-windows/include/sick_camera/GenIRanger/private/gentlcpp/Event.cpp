// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Event.h"
#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Module.h"
#include "../../public/gentlcpp/Tracing.h"

#include <iostream>

namespace gentlcpp {

Event::~Event()
{
  trace("~Event()");
  mCApi->GCUnregisterEvent(mEventSrcHandle, mEventType);
}

void Event::flush()
{
  ThrowIfError(mCApi, mCApi->EventFlush(mEventHandle));
}

GenTL::EVENT_HANDLE Event::getHandle()
{
  return mEventHandle;
}

std::shared_ptr<const Module> Event::getParentModule() const
{
  return mParentModule;
}

size_t Event::getData(void* pOutBuffer,
                      size_t bufferSize,
                      std::chrono::milliseconds timeout) const
{
  ThrowIfError(mCApi,
               mCApi->EventGetData(
                 mEventHandle, pOutBuffer, &bufferSize, timeout.count()));
  return bufferSize;
}

size_t Event::getData(void* pOutBuffer, size_t bufferSize) const
{
  ThrowIfError(
    mCApi,
    mCApi->EventGetData(mEventHandle, pOutBuffer, &bufferSize, GENTL_INFINITE));
  return bufferSize;
}


GenTL::EVENT_TYPE_LIST Event::getEventType() const
{
  return mEventType;
}

size_t Event::getQueuedCount() const
{
  return getInfo<size_t>(GenTL::EVENT_NUM_IN_QUEUE);
}

uint64_t Event::getFiredCount() const
{
  return getInfo<uint64_t>(GenTL::EVENT_NUM_FIRED);
}

size_t Event::getMaxSize() const
{
  return getInfo<size_t>(GenTL::EVENT_SIZE_MAX);
}

size_t Event::getInfoDataMaxSize() const
{
  return getInfo<size_t>(GenTL::EVENT_INFO_DATA_SIZE_MAX);
}

uint64_t Event::getDataNumericId(const void* pEventBuffer,
                                 size_t eventBufferSize) const
{
  return getDataInfo<uint64_t>(
    pEventBuffer, eventBufferSize, GenTL::EVENT_DATA_NUMID);
}

template<>
std::string Event::getInfo(GenTL::EVENT_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->EventGetInfo, mEventHandle, command, &dataType);
}

template<>
std::string Event::getDataInfo(const void* pEventBuffer,
                               size_t eventBufferSize,
                               GenTL::EVENT_DATA_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->EventGetDataInfo,
                   mEventHandle,
                   pEventBuffer,
                   eventBufferSize,
                   command,
                   &dataType);
}



Event::Event(std::shared_ptr<const CApi> cApi,
             std::shared_ptr<const Module> module,
             GenTL::EVENT_TYPE_LIST eventType,
             GenTL::EVENTSRC_HANDLE eventSrcHandle)
  : mCApi(cApi)
  , mParentModule(module)
  , mEventType(eventType)
  , mEventHandle(GENTL_INVALID_HANDLE)
  , mEventSrcHandle(eventSrcHandle)
{
  gentlcpp::trace("Event()");
  ThrowIfError(
    mCApi, mCApi->GCRegisterEvent(eventSrcHandle, eventType, &mEventHandle));
}

}
