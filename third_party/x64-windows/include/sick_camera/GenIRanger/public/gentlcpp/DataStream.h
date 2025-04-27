// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "BasicTypes.h"
#include "Module.h"
#include "CApi.h"

#include <TLI/GenTL.h>
#include <memory>

namespace gentlcpp {

class Buffer;
class Device;
class CApi;
class NewBufferEvent;
class Port;

/**
 * Represents a data stream from a device in the GenTL data stream module.
 *
 * See \ref Device::openDataStream to get an instance.
 */
#ifdef SWIG
class DataStream : public Module
#else
class DataStream : public std::enable_shared_from_this<DataStream>,
                   public Module
#endif
{
public:
   // ~DataStream() noexcept;
    ~DataStream() noexcept;

  GenTL::DS_HANDLE getHandle();

  std::shared_ptr<Event> registerEvent(GenTL::EVENT_TYPE_LIST eventType);
  std::shared_ptr<NewBufferEvent> registerNewBufferEvent();

  std::shared_ptr<Port> getPort();

  std::shared_ptr<Buffer> announceBuffer(
    void* pOutBuffer, size_t size, BufferUserData userPointer);
  BufferUserData revokeBuffer(std::shared_ptr<const Buffer> buffer);
  /** Queues the buffer with the option to clear it or not */
  void queueBuffer(std::shared_ptr<Buffer> buffer, bool clearBuffer = true);

  void flushQueue(GenTL::ACQ_QUEUE_TYPE_LIST operation);

  void startAcquisition(GenTL::ACQ_START_FLAGS_LIST startFlags,
                        int64_t numToAcquire);
  void stopAcquisition(GenTL::ACQ_STOP_FLAGS_LIST stopFlags);

  DataStreamId getId() const;
  uint64_t getDeliveredCount() const;
  uint64_t getUnderrunCount() const;
  size_t getAnnouncedCount() const;
  size_t getQueuedCount() const;
  size_t getAwaitingDeliveryCount() const;
  uint64_t getStartedCount() const;
  size_t getPayloadSize() const;
  bool isGrabbing() const;
  bool definesPayloadSize() const;
  std::string getTransportLayerType() const;
  size_t getMaxChunkCount() const;
  size_t getMinBufferAnnounceCount() const;
  size_t getBufferAlignment() const;

  template<typename T>
  T getInfo(GenTL::STREAM_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(T);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi->DSGetInfo(mHandle, command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::STREAM_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(static_cast<GenTL::STREAM_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

private:
  friend class Device;

  DataStream(std::shared_ptr<const CApi> cApi,
             std::shared_ptr<Device> device,
             const DataStreamId& dataStreamId);
  DataStream(const DataStream&) = delete;

private:
  // Hold a reference to parent to make sure objects are destructed in correct
  // order
  std::shared_ptr<const Device> mParent;
  GenTL::DS_HANDLE mHandle;
};

}
