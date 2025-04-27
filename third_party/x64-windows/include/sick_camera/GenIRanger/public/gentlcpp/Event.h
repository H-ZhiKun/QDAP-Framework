// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "CApi.h"

#include <TLI/GenTL.h>
#include <chrono>
#include <memory>
#include <string>

namespace gentlcpp {

class CApi;
class Module;

/**
 * Instances of this class represent a specific GenTL event type. To get
 * notified when an event is signaled the \ref Event::getData function should be
 * called.
 *
 * Event objects can be retrieved from the corresponding module object, for
 * instance, \ref Device::registerEvent.
 */
class Event
{
public:
  ~Event() noexcept;

  void flush();

  GenTL::EVENT_HANDLE getHandle();
  std::shared_ptr<const Module> getParentModule() const;

  /**
   * Wait for the event and fill pOutBuffer with the returned event data
   *
   * \return the number of bytes that were written to pOutBuffer
   */
  size_t getData(void* pOutBuffer,
                 size_t bufferSize,
                 std::chrono::milliseconds timeout) const;
  /**
   * Wait for the event with an infinite timeout and fill pOutBuffer with the
   * returned event data
   *
   * \return the number of bytes that were written to pOutBuffer
   */
  size_t getData(void* pOutBuffer, size_t bufferSize) const;

  GenTL::EVENT_TYPE_LIST getEventType() const;
  size_t getQueuedCount() const;
  uint64_t getFiredCount() const;
  size_t getMaxSize() const;
  size_t getInfoDataMaxSize() const;
  uint64_t getDataNumericId(const void* pEventBuffer,
                            size_t eventBufferSize) const;

  template<typename T>
  T getInfo(GenTL::EVENT_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(value);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(
      mCApi,
      mCApi->EventGetInfo(mEventHandle, command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::EVENT_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(static_cast<GenTL::EVENT_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

  template<typename T>
  T getDataInfo(const void* pEventBuffer,
                size_t eventBufferSize,
                GenTL::EVENT_DATA_INFO_CMD_LIST command) const
  {
    T value;
    size_t outSize = sizeof(value);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi,
                 mCApi->EventGetDataInfo(mEventHandle,
                                         pEventBuffer,
                                         eventBufferSize,
                                         command,
                                         &dataType,
                                         &value,
                                         &outSize));
    return value;
  }

  template<>
  std::string getDataInfo(const void* pEventBuffer,
                          size_t eventBufferSize,
                          GenTL::EVENT_DATA_INFO_CMD_LIST command) const;

  template<typename T>
  T getDataInfo(const void* pEventBuffer,
                size_t eventBufferSize,
                uint32_t customCommand) const
  {
    return getDataInfo<T>(
      pEventBuffer,
      eventBufferSize,
      static_cast<GenTL::EVENT_DATA_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getDataInfo);
#endif

protected:
  friend class DataStream;
  friend class Device;
  friend class Interface;
  friend class TransportLayer;

  Event(std::shared_ptr<const CApi> cApi,
        std::shared_ptr<const Module> module,
        GenTL::EVENT_TYPE_LIST eventType,
        GenTL::EVENTSRC_HANDLE eventSrcHandle);

protected:
  std::shared_ptr<const CApi> mCApi;

private:
  std::shared_ptr<const Module> mParentModule;
  GenTL::EVENT_TYPE_LIST mEventType;
  GenTL::EVENT_HANDLE mEventHandle;
  GenTL::EVENTSRC_HANDLE mEventSrcHandle;
};

}
