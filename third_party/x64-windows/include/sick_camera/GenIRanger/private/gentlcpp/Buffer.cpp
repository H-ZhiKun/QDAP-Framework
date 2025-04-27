// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Buffer.h"

#include "GetString.h"
#include "../../public/gentlcpp/BufferPart.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/DataStream.h"
#include "../../public/gentlcpp/Tracing.h"

namespace gentlcpp {

Buffer::~Buffer()
{
  trace("~Buffer()");
}

void Buffer::clear()
{
  std::memset(reinterpret_cast<void*>(getBase()), 0, getSize());
}

GenTL::BUFFER_HANDLE Buffer::getHandle()
{
  return mHandle;
}

BufferUserData Buffer::getUserData() const
{
  return mUserData;
}

GenTL::PAYLOADTYPE_INFO_IDS Buffer::getPayloadType() const
{
  size_t type = getInfo<size_t>(GenTL::BUFFER_INFO_PAYLOADTYPE);
  return static_cast<GenTL::PAYLOADTYPE_INFO_IDS>(type);
}

bool Buffer::containsChunkData() const
{
  return getInfo<bool8_t>(GenTL::BUFFER_INFO_CONTAINS_CHUNKDATA);
}

size_t Buffer::getDeliveredChunkPayloadSize() const
{
  return getInfo<size_t>(GenTL::BUFFER_INFO_DELIVERED_CHUNKPAYLOADSIZE);
}

BufferPointerType Buffer::getBase()
{
  return getInfo<BufferPointerType>(GenTL::BUFFER_INFO_BASE);
}

size_t Buffer::getSize() const
{
  return getInfo<size_t>(GenTL::BUFFER_INFO_SIZE);
}

size_t Buffer::getSizeFilled() const
{
  return getInfo<size_t>(GenTL::BUFFER_INFO_SIZE_FILLED);
}

size_t Buffer::getDataSize() const
{
  return getInfo<size_t>(GenTL::BUFFER_INFO_DATA_SIZE);
}

uint64_t Buffer::getFrameId() const
{
  return getInfo<uint64_t>(GenTL::BUFFER_INFO_FRAMEID);
}

uint64_t Buffer::getTimeStamp() const
{
  return getInfo<uint64_t>(GenTL::BUFFER_INFO_TIMESTAMP);
}

size_t Buffer::getWidth() const
{
  return getInfo<size_t>(GenTL::BUFFER_INFO_WIDTH);
}

size_t Buffer::getHeight() const
{
  return getInfo<size_t>(GenTL::BUFFER_INFO_HEIGHT);
}

uint64_t Buffer::getPixelFormat() const
{
  return getInfo<uint64_t>(GenTL::BUFFER_INFO_PIXELFORMAT);
}

bool Buffer::hasNewData() const
{
  return getInfo<bool8_t>(GenTL::BUFFER_INFO_NEW_DATA);
}

bool Buffer::isQueued() const
{
  return getInfo<bool8_t>(GenTL::BUFFER_INFO_IS_QUEUED);
}

bool Buffer::isAcquiring() const
{
  return getInfo<bool8_t>(GenTL::BUFFER_INFO_IS_ACQUIRING);
}

bool Buffer::isIncomplete() const
{
  return getInfo<bool8_t>(GenTL::BUFFER_INFO_IS_INCOMPLETE);
}

std::string Buffer::getTransportLayerType() const
{
  return getInfo<std::string>(GenTL::BUFFER_INFO_TLTYPE);
}

uint32_t Buffer::getBufferPartCount() const
{
  uint32_t numParts = 0;
  ThrowIfError(
    mCApi,
    mCApi->DSGetNumBufferParts(mParent->getHandle(), mHandle, &numParts));
  return numParts;
}

std::shared_ptr<BufferPart> Buffer::getBufferPart(uint32_t index)
{
  return std::shared_ptr<BufferPart>(
    new BufferPart(mCApi, shared_from_this(), index));
}

template<>
std::string Buffer::getInfo(GenTL::BUFFER_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(
    mCApi->DSGetBufferInfo, mParent->getHandle(), mHandle, command, &dataType);
}

Buffer::Buffer(std::shared_ptr<const CApi> cApi,
               std::shared_ptr<DataStream> dataStream,
               GenTL::BUFFER_HANDLE bufferHandle,
               BufferUserData userData)
  : mCApi(cApi)
  , mParent(dataStream)
  , mHandle(bufferHandle)
  , mUserData(userData)
{
  trace("Buffer()");
}

}
