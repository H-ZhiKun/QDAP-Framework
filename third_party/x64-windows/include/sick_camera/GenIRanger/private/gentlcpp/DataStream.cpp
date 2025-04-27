// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/DataStream.h"

#include "GetString.h"
#include "../../public/gentlcpp/Buffer.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Device.h"
#include "../../public/gentlcpp/Event.h"
#include "../../public/gentlcpp/NewBufferEvent.h"
#include "../../public/gentlcpp/Port.h"
#include "../../public/gentlcpp/Tracing.h"

namespace gentlcpp {

DataStream::~DataStream()
{
  trace("~DataStream()");
  mCApi->DSClose(mHandle);
}

GenTL::DS_HANDLE DataStream::getHandle()
{
  return mHandle;
}

std::shared_ptr<Event>
DataStream::registerEvent(GenTL::EVENT_TYPE_LIST eventType)
{
  return std::shared_ptr<Event>(
    new Event(mCApi, shared_from_this(), eventType, mHandle));
}

std::shared_ptr<NewBufferEvent> DataStream::registerNewBufferEvent()
{
  return std::shared_ptr<NewBufferEvent>(new NewBufferEvent(
    mCApi, shared_from_this(), GenTL::EVENT_NEW_BUFFER, mHandle));
}

std::shared_ptr<Port> DataStream::getPort()
{
  return std::shared_ptr<Port>(new Port(mCApi, shared_from_this(), mHandle));
}

std::shared_ptr<Buffer> DataStream::announceBuffer(void* pOutBuffer,
                                                   size_t size,
                                                   BufferUserData userPointer)
{
  GenTL::BUFFER_HANDLE bufferHandle = GENTL_INVALID_HANDLE;
  ThrowIfError(mCApi->DSAnnounceBuffer(
    mHandle, pOutBuffer, size, userPointer, &bufferHandle));
  return std::shared_ptr<Buffer>(
    new Buffer(mCApi, shared_from_this(), bufferHandle, userPointer));
}

gentlcpp::BufferUserData
DataStream::revokeBuffer(std::shared_ptr<const Buffer> buffer)
{
  void* bufferData = nullptr;
  BufferUserData userData = nullptr;
  ThrowIfError(
    mCApi->DSRevokeBuffer(mHandle, buffer->mHandle, &bufferData, &userData));
  return userData;
}

void DataStream::queueBuffer(std::shared_ptr<Buffer> buffer, bool clearBuffer)
{
  if (clearBuffer)
  {
    buffer->clear();
  }
  ThrowIfError(mCApi->DSQueueBuffer(mHandle, buffer->mHandle));
}

void DataStream::flushQueue(GenTL::ACQ_QUEUE_TYPE_LIST operation)
{
  ThrowIfError(mCApi->DSFlushQueue(mHandle, operation));
}

void DataStream::startAcquisition(GenTL::ACQ_START_FLAGS_LIST startFlags,
                                  int64_t numToAcquire)
{
  ThrowIfError(mCApi->DSStartAcquisition(mHandle, startFlags, numToAcquire));
}

void DataStream::stopAcquisition(GenTL::ACQ_STOP_FLAGS_LIST stopFlags)
{
  ThrowIfError(mCApi->DSStopAcquisition(mHandle, stopFlags));
}

gentlcpp::DataStreamId DataStream::getId() const
{
  return getInfo<std::string>(GenTL::STREAM_INFO_ID);
}

uint64_t DataStream::getDeliveredCount() const
{
  return getInfo<uint64_t>(GenTL::STREAM_INFO_NUM_DELIVERED);
}

uint64_t DataStream::getUnderrunCount() const
{
  return getInfo<uint64_t>(GenTL::STREAM_INFO_NUM_UNDERRUN);
}

size_t DataStream::getAnnouncedCount() const
{
  return getInfo<size_t>(GenTL::STREAM_INFO_NUM_ANNOUNCED);
}

size_t DataStream::getQueuedCount() const
{
  return getInfo<size_t>(GenTL::STREAM_INFO_NUM_QUEUED);
}

size_t DataStream::getAwaitingDeliveryCount() const
{
  return getInfo<size_t>(GenTL::STREAM_INFO_NUM_AWAIT_DELIVERY);
}

uint64_t DataStream::getStartedCount() const
{
  return getInfo<uint64_t>(GenTL::STREAM_INFO_NUM_STARTED);
}

size_t DataStream::getPayloadSize() const
{
  return getInfo<size_t>(GenTL::STREAM_INFO_PAYLOAD_SIZE);
}

bool DataStream::isGrabbing() const
{
  return getInfo<bool8_t>(GenTL::STREAM_INFO_IS_GRABBING);
}

bool DataStream::definesPayloadSize() const
{
  return getInfo<bool8_t>(GenTL::STREAM_INFO_DEFINES_PAYLOADSIZE);
}

std::string DataStream::getTransportLayerType() const
{
  return getInfo<std::string>(GenTL::STREAM_INFO_TLTYPE);
}

size_t DataStream::getMaxChunkCount() const
{
  return getInfo<size_t>(GenTL::STREAM_INFO_NUM_CHUNKS_MAX);
}

size_t DataStream::getMinBufferAnnounceCount() const
{
  return getInfo<size_t>(GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN);
}

size_t DataStream::getBufferAlignment() const
{
  return getInfo<size_t>(GenTL::STREAM_INFO_BUF_ALIGNMENT);
}

template<>
std::string DataStream::getInfo(GenTL::STREAM_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->DSGetInfo, mHandle, command, &dataType);
}

DataStream::DataStream(std::shared_ptr<const CApi> cApi,
                       std::shared_ptr<Device> device,
                       const DataStreamId& dataStreamId)
  : Module(cApi)
  , mParent(device)
  , mHandle(GENTL_INVALID_HANDLE)
{
  trace("DataStream()");
  ThrowIfError(mCApi->DevOpenDataStream(
    device->getHandle(), dataStreamId.c_str(), &mHandle));
}

}
