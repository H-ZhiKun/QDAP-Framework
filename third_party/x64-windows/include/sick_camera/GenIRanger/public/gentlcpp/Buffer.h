// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "../../public/gentlcpp/DataStream.h"

#include <TLI/GenTL.h>

#include <cstdint>
#include <memory>
#include <string>

namespace gentlcpp {

class BufferPart;
class DataStream;
class CApi;

/**
 * Represents a buffer in the GenTL buffer module.
 *
 * See \ref DataStream::announceBuffer to get an instance.
 */
#ifdef SWIG
class Buffer
#else
class Buffer : public std::enable_shared_from_this<Buffer>
#endif
{
public:
    ~Buffer() noexcept; //noexcept;

  /** Clears all data from the allocated buffer */
  void clear();

  GenTL::BUFFER_HANDLE getHandle();

  BufferUserData getUserData() const;
  GenTL::PAYLOADTYPE_INFO_IDS getPayloadType() const;
  bool containsChunkData() const;
  size_t getDeliveredChunkPayloadSize() const;
  BufferPointerType getBase();
  size_t getSize() const;
  size_t getSizeFilled() const;
  size_t getDataSize() const;
  uint64_t getFrameId() const;
  uint64_t getTimeStamp() const;
  size_t getWidth() const;
  size_t getHeight() const;
  uint64_t getPixelFormat() const;
  bool hasNewData() const;
  bool isQueued() const;
  bool isAcquiring() const;
  bool isIncomplete() const;
  std::string getTransportLayerType() const;

  // Multi-part functionality
  uint32_t getBufferPartCount() const;
  std::shared_ptr<BufferPart> getBufferPart(uint32_t partIndex);

  template<typename T>
  T getInfo(GenTL::BUFFER_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(T);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(
      mCApi,
      mCApi->DSGetBufferInfo(
        mParent->getHandle(), mHandle, command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::BUFFER_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(static_cast<GenTL::BUFFER_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

private:
  friend class DataStream;
  friend class BufferPart;

  Buffer(std::shared_ptr<const CApi> cApi,
         std::shared_ptr<DataStream> dataStream,
         GenTL::BUFFER_HANDLE bufferHandle,
         BufferUserData userData);
  Buffer(const Buffer&) = delete;

private:
  std::shared_ptr<const CApi> mCApi;
  std::shared_ptr<DataStream> mParent;
  GenTL::BUFFER_HANDLE mHandle;
  BufferUserData mUserData;
};

}
