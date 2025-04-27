// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "../../public/gentlcpp/Buffer.h"

#include <TLI/GenTL.h>

#include <cstdint>
#include <memory>
#include <string>

namespace gentlcpp {

/**
 * Represents a single part of a multi-part buffer in the GenTL buffer module.
 *
 * See \ref Buffer::getBufferPart to get an instance.
 */
class BufferPart
{
public:
  ~BufferPart() noexcept;

  uint32_t getIndex() const;
  BufferPointerType getBase();
  /**
   * \return the offset in bytes from the start of the buffer to the start of
   *         this part
   */
  ptrdiff_t getDataOffset();
  size_t getDataSize();
  GenTL::PARTDATATYPE_IDS getDataType() const;
  uint64_t getDataFormat() const;
  GenTL::PIXELFORMAT_NAMESPACE_IDS getDataFormatNamespace() const;
  size_t getWidth() const;
  size_t getHeight() const;
  size_t getXOffset() const;
  size_t getYOffset() const;
  size_t getXPadding() const;
  uint64_t getSourceId() const;
  size_t getDeliveredImageHeight() const;

  template<typename T>
  T getInfo(GenTL::BUFFER_PART_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(T);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi,
                 mCApi->DSGetBufferPartInfo(mParent->mParent->getHandle(),
                                            mParent->getHandle(),
                                            mPartIndex,
                                            command,
                                            &dataType,
                                            &value,
                                            &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::BUFFER_PART_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(
      static_cast<GenTL::BUFFER_PART_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

private:
  friend class Buffer;

  BufferPart(std::shared_ptr<const CApi> cApi,
             std::shared_ptr<Buffer> buffer,
             uint32_t partIndex);
  BufferPart(const BufferPart&) = delete;

private:
  std::shared_ptr<const CApi> mCApi;
  std::shared_ptr<Buffer> mParent;
  uint32_t mPartIndex;
};

}
