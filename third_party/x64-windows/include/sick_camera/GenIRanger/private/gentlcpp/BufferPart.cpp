// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/BufferPart.h"

#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/DataStream.h"
#include "../../public/gentlcpp/Tracing.h"

namespace gentlcpp {

BufferPart::~BufferPart()
{
  trace("~BufferPart()");
}

uint32_t BufferPart::getIndex() const
{
  return mPartIndex;
}

BufferPointerType BufferPart::getBase()
{
  return getInfo<BufferPointerType>(GenTL::BUFFER_INFO_BASE);
}

ptrdiff_t BufferPart::getDataOffset()
{
  ptrdiff_t offset = getBase() - mParent->getBase();
  return offset;
}

size_t BufferPart::getDataSize()
{
  return getInfo<size_t>(GenTL::BUFFER_PART_INFO_DATA_SIZE);
}

GenTL::PARTDATATYPE_IDS BufferPart::getDataType() const
{
  size_t dataType = getInfo<size_t>(GenTL::BUFFER_PART_INFO_DATA_TYPE);
  return static_cast<GenTL::PARTDATATYPE_IDS>(dataType);
}

uint64_t BufferPart::getDataFormat() const
{
  return getInfo<uint64_t>(GenTL::BUFFER_PART_INFO_DATA_FORMAT);
}

GenTL::PIXELFORMAT_NAMESPACE_IDS BufferPart::getDataFormatNamespace() const
{
  uint64_t format =
    getInfo<uint64_t>(GenTL::BUFFER_PART_INFO_DATA_FORMAT_NAMESPACE);
  return static_cast<GenTL::PIXELFORMAT_NAMESPACE_IDS>(format);
}

size_t BufferPart::getWidth() const
{
  return getInfo<size_t>(GenTL::BUFFER_PART_INFO_WIDTH);
}

size_t BufferPart::getHeight() const
{
  return getInfo<size_t>(GenTL::BUFFER_PART_INFO_HEIGHT);
}

size_t BufferPart::getXOffset() const
{
  return getInfo<size_t>(GenTL::BUFFER_PART_INFO_XOFFSET);
}

size_t BufferPart::getYOffset() const
{
  return getInfo<size_t>(GenTL::BUFFER_PART_INFO_YOFFSET);
}

size_t BufferPart::getXPadding() const
{
  return getInfo<size_t>(GenTL::BUFFER_PART_INFO_XPADDING);
}

uint64_t BufferPart::getSourceId() const
{
  return getInfo<uint64_t>(GenTL::BUFFER_PART_INFO_SOURCE_ID);
}

size_t BufferPart::getDeliveredImageHeight() const
{
  return getInfo<size_t>(GenTL::BUFFER_PART_INFO_DELIVERED_IMAGEHEIGHT);
}

template<>
std::string BufferPart::getInfo(GenTL::BUFFER_PART_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->DSGetBufferPartInfo,
                   mParent->mParent->getHandle(),
                   mParent->getHandle(),
                   mPartIndex,
                   command,
                   &dataType);
}

BufferPart::BufferPart(std::shared_ptr<const CApi> cApi,
                       std::shared_ptr<Buffer> buffer,
                       uint32_t partIndex)
  : mCApi(cApi)
  , mParent(buffer)
  , mPartIndex(partIndex)
{
  trace("BufferPart()");
}

}
