// Copyright 2019-2020 SICK AG. All rights reserved.
#include "GenTlBackedFrame.h"

#include "../EnumSelectorEntries.h"
#include "../GenIUtil.h"
#include "../../public/Exceptions.h"
#include "../../public/GenIRanger.h"
#include "../../public/gentlcpp/Buffer.h"
#include "../../public/gentlcpp/BufferPart.h"
#include "../../public/gentlcpp/CApi.h"

namespace {

typedef uint32_t BufferPartIndex;

/**
 * Returns the ComponentId of a GenTL buffer part.
 *
 * This is done by first looking up the component name, via the device node map,
 * and then mapping the name to a geniranger enum.
 *
 * The enum value via GenTL (sent from the device) does not necessarily match
 * the enum value in GenIRanger.
 */
geniranger::frame::ComponentId
getComponentId(const geniranger::EnumSelectorEntries& componentLookup,
               std::shared_ptr<gentlcpp::Buffer> buffer,
               BufferPartIndex index)
{
  uint64_t gentlComponentId = buffer->getBufferPart(index)->getInfo<uint64_t>(
    geniranger::BUFFER_PART_CUSTOM_INFO_CMD_LIST::
      BUFFER_PART_CUSTOM_INFO_DATA_PURPOSE_ID);

  const std::string& genapiName = componentLookup.name(gentlComponentId);
  return geniranger::frame::componentIdFromString(genapiName);
}

/**
 * Returns the RegionId of a GenTL buffer part.
 *
 * This is done by first looking up the region name, via the device node map,
 * and then mapping the name to a geniranger enum.
 *
 * The enum value via GenTL (sent from the device) does not necessarily match
 * the enum value in GenIRanger.
 */
geniranger::frame::RegionId
getRegionId(const geniranger::EnumSelectorEntries& regionLookup,
            std::shared_ptr<gentlcpp::Buffer> buffer,
            BufferPartIndex index)
{
  uint64_t gentlRegionId = buffer->getBufferPart(index)->getInfo<uint64_t>(
    geniranger::BUFFER_PART_CUSTOM_INFO_CMD_LIST::
      BUFFER_PART_CUSTOM_INFO_REGION_ID);

  const std::string& genapiName = regionLookup.name(gentlRegionId);
  return geniranger::frame::regionIdFromString(genapiName);
}

}


namespace geniranger { namespace frame {

GenTlBackedFrame::GenTlBackedFrame(std::shared_ptr<gentlcpp::Buffer> buffer,
                                   const EnumSelectorEntries& regionLookup,
                                   const EnumSelectorEntries& componentLookup,
                                   std::shared_ptr<LineMetadata> lineMetadata,
                                   const FrameConfigurationInfo& frameInfo)
  : mBuffer(buffer)
  , mLineMetadata(lineMetadata)
{
  if (buffer->getPayloadType() == GenTL::PAYLOAD_TYPE_MULTI_PART)
  {
    initMultiPart(buffer, regionLookup, componentLookup, frameInfo);
  }
  else if (buffer->getPayloadType() == GenTL::PAYLOAD_TYPE_IMAGE)
  {
    if (frameInfo.deviceScanType == DeviceScanType::AREASCAN)
    {
      initSinglePartGray(buffer, frameInfo.regions.at(RegionId::REGION_0));
    }
    else
    {
      initSinglePart3d(buffer,
                       frameInfo.regions.at(RegionId::SCAN_3D_EXTRACTION_1));
    }
  }
  else
  {
    std::ostringstream ss;
    ss << "Unsupported payload type: " << buffer->getPayloadType();
    std::string message = ss.str();
    throw GenIRangerException(message);
  }
}

ConstRegionList GenTlBackedFrame::getRegions() const
{
  return geniutil::constPointerVector(mRegions);
}

bool GenTlBackedFrame::hasRegion(RegionId regionId) const
{
  return find(regionId) != nullptr;
}

std::shared_ptr<const Region>
GenTlBackedFrame::getRegion(RegionId regionId) const
{
  std::shared_ptr<Region> result = find(regionId);
  if (result == nullptr)
  {
    std::ostringstream ss;
    ss << "No region with id: " << static_cast<uint64_t>(regionId);
    std::string message = ss.str();
    throw std::invalid_argument(message);
  }
  return result;
}

std::shared_ptr<const LineMetadata> GenTlBackedFrame::getLineMetadata() const
{
  return mLineMetadata;
}

uint64_t GenTlBackedFrame::getFrameId() const
{
  return mBuffer->getFrameId();
}

bool GenTlBackedFrame::isIncomplete() const
{
  return mBuffer->isIncomplete();
}

void GenTlBackedFrame::save(const std::string& filePath)
{
  if (mRegions.size() > 1)
  {
    // TODO: RNX-4506
    throw std::invalid_argument(
      "IFrame does not yet handle saving more than a single region");
  }

  if (find(RegionId::REGION_0))
  {
    std::shared_ptr<const Region> grayRegion = getRegion(RegionId::REGION_0);
    std::shared_ptr<const Component> intensityComponent =
      grayRegion->getComponent(ComponentId::INTENSITY);
    // Second filePath argument to add file name as extra information in XML
    saveBuffer8(intensityComponent->getData(),
                intensityComponent->getWidth(),
                intensityComponent->getHeight(),
                grayRegion->getAoi().offsetX,
                grayRegion->getAoi().offsetY,
                filePath,
                filePath);
  }
  else if (find(RegionId::SCAN_3D_EXTRACTION_1))
  {
    saveLinescan3d(filePath);
  }
  else
  {
    // This shouldn't happen
    throw std::logic_error("The frame unexpectedly didn't contain either "
                           "Region0 or Scan3DExtraction1");
  }
}

std::shared_ptr<gentlcpp::Buffer> GenTlBackedFrame::getBuffer()
{
  return mBuffer;
}

void GenTlBackedFrame::logTo(std::ostream& s) const
{
  s << "Frame ID: " << getFrameId() << std::endl;
  s << "Incomplete: " << isIncomplete() << std::endl;
  for (auto region : this->getRegions())
  {
    s << *region;
  }
}

void GenTlBackedFrame::initMultiPart(std::shared_ptr<gentlcpp::Buffer> buffer,
                                     const EnumSelectorEntries& regionLookup,
                                     const EnumSelectorEntries& componentLookup,
                                     const FrameConfigurationInfo& frameInfo)
{
  const uint32_t bufferPartCount = buffer->getBufferPartCount();

  for (BufferPartIndex index = 0; index < bufferPartCount; ++index)
  {
    RegionId regionId = getRegionId(regionLookup, buffer, index);
    const RegionConfigurationInfo& regionInfo = frameInfo.regions.at(regionId);
    std::shared_ptr<Region> region = findOrAdd(regionId, regionInfo);
    std::shared_ptr<gentlcpp::BufferPart> part = buffer->getBufferPart(index);
    PixelFormat pixelFormat = static_cast<PixelFormat>(part->getDataFormat());

    std::shared_ptr<Component> component =
      region->findOrAdd(getComponentId(componentLookup, buffer, index),
                        pixelFormat,
                        static_cast<uint16_t>(part->getWidth()),
                        static_cast<uint16_t>(part->getHeight()),
                        reinterpret_cast<uint8_t*>(part->getBase()),
                        part->getDataSize());
    component->mBufferPart = part;
  }
}

void GenTlBackedFrame::initSinglePartGray(
  std::shared_ptr<gentlcpp::Buffer> buffer,
  const RegionConfigurationInfo& regionInfo)
{
  std::shared_ptr<Region> region = findOrAdd(RegionId::REGION_0, regionInfo);
  PixelFormat pixelFormat = static_cast<PixelFormat>(buffer->getPixelFormat());
  std::shared_ptr<Component> component =
    region->findOrAdd(ComponentId::INTENSITY,
                      pixelFormat,
                      static_cast<uint16_t>(buffer->getWidth()),
                      static_cast<uint16_t>(buffer->getHeight()),
                      reinterpret_cast<uint8_t*>(buffer->getBase()),
                      buffer->getDataSize());
}

void GenTlBackedFrame::initSinglePart3d(
  std::shared_ptr<gentlcpp::Buffer> buffer,
  const RegionConfigurationInfo& regionInfo)
{
  std::shared_ptr<Region> region =
    findOrAdd(RegionId::SCAN_3D_EXTRACTION_1, regionInfo);
  PixelFormat pixelFormat = static_cast<PixelFormat>(buffer->getPixelFormat());
  std::shared_ptr<Component> component =
    region->findOrAdd(ComponentId::RANGE,
                      pixelFormat,
                      static_cast<uint16_t>(buffer->getWidth()),
                      static_cast<uint16_t>(buffer->getHeight()),
                      reinterpret_cast<uint8_t*>(buffer->getBase()),
                      buffer->getDataSize());
}

std::shared_ptr<Region> GenTlBackedFrame::find(RegionId regionId) const
{
  for (auto region : mRegions)
  {
    if (region->getId() == regionId)
    {
      return region;
    }
  }
  return std::shared_ptr<Region>();
}

std::shared_ptr<Region>
GenTlBackedFrame::findOrAdd(RegionId regionId,
                            const RegionConfigurationInfo& regionInfo)
{
  auto region = find(regionId);
  if (!region)
  {
    mRegions.push_back(
      std::shared_ptr<Region>(new Region(regionId, regionInfo)));
    region = mRegions.back();
  }
  return region;
}

void GenTlBackedFrame::setAoi(RegionPtr& legacyRegion,
                              std::shared_ptr<const Region> region)
{
  Aoi aoi = region->mAoi;
  legacyRegion->aoiOffset(aoi.offsetX, aoi.offsetY)
    .aoiSize(aoi.width, aoi.height)
    .rangeAxis(region->mRangeAxis);
}

void GenTlBackedFrame::setRangeComponent(RegionPtr& legacyRegion,
                                         std::shared_ptr<const Region> region)
{
  std::shared_ptr<const Component> rangeComponent =
    region->getComponent(ComponentId::RANGE);

  size_t buffer16Size =
    rangeComponent->getWidth() * rangeComponent->getHeight() * sizeof(uint16_t);
  legacyRegion->createRange(buffer16Size, PixelWidth::PW16);

  // Convert 12-bit into 16-bit, since this is required by the ICon format
  if (rangeComponent->getPixelFormat() == PixelFormat::COORD_3D_C12P
      || rangeComponent->getPixelFormat() == PixelFormat::MONO_12P)
  {
    legacyRegion->range()->data().reserve(buffer16Size);
    int64_t convertedSize =
      static_cast<int64_t>(legacyRegion->range()->data().size());
    convert12pTo16(rangeComponent->getData(),
                   rangeComponent->getWidth() * rangeComponent->getHeight() * 12
                     / 8,
                   legacyRegion->range()->data().data(),
                   &convertedSize);
  }
  else
  {
    // Just copy the data as-is
    uint8_t* pBuffer16 = rangeComponent->getData();
    legacyRegion->range()->data().assign(pBuffer16, pBuffer16 + buffer16Size);
  }
}

void GenTlBackedFrame::setReflectanceComponent(
  RegionPtr& legacyRegion,
  std::shared_ptr<const Region> region)
{
  std::shared_ptr<const Component> reflectanceComponent =
    region->getComponent(ComponentId::REFLECTANCE);
  size_t bufferSize = reflectanceComponent->getWidth()
                      * reflectanceComponent->getHeight() * sizeof(uint8_t);
  legacyRegion->createReflectance(bufferSize, PixelWidth::PW8);
  uint8_t* pBuffer8 = reflectanceComponent->getData();
  legacyRegion->reflectance()->data().assign(pBuffer8, pBuffer8 + bufferSize);
}

void GenTlBackedFrame::setScatterComponent(RegionPtr& legacyRegion,
                                           std::shared_ptr<const Region> region)
{
  std::shared_ptr<const Component> scatterComponent =
    region->getComponent(ComponentId::SCATTER);

  PixelWidth pixelWidth;
  switch (scatterComponent->getPixelFormat())
  {
  case PixelFormat::MONO_8: pixelWidth = PixelWidth::PW8; break;
  case PixelFormat::MONO_16: pixelWidth = PixelWidth::PW16; break;
  default:
    throw GenIRangerException(
      "Scatter only supports Mono8 or Mono16 pixel formats");
  }

  size_t bufferSize = scatterComponent->getWidth()
                      * scatterComponent->getHeight()
                      * (static_cast<size_t>(pixelWidth) / 8);
  legacyRegion->createScatter(bufferSize, pixelWidth);
  uint8_t* pBuffer8 = scatterComponent->getData();
  legacyRegion->scatter()->data().assign(pBuffer8, pBuffer8 + bufferSize);
}

RegionPtr GenTlBackedFrame::createLegacyLinescan3dRegion(
  Frame& frame,
  std::shared_ptr<const Region> region)
{
  // TODO RNX-5174: We are still using the frame abstraction for the actual
  // save function. However, the GUI cannot yet use this new abstraction
  RegionPtr legacyRegion = frame.createRegion();

  setAoi(legacyRegion, region);
  setRangeComponent(legacyRegion, region);
  if (region->find(ComponentId::REFLECTANCE))
  {
    setReflectanceComponent(legacyRegion, region);
  }
  if (region->find(ComponentId::SCATTER))
  {
    setScatterComponent(legacyRegion, region);
  }

  return legacyRegion;
}

void GenTlBackedFrame::saveLinescan3d(const std::string& filePath)
{
  Frame toBeSavedFrame;

  RegionPtr scan3dExtraction1 = createLegacyLinescan3dRegion(
    toBeSavedFrame, getRegion(RegionId::SCAN_3D_EXTRACTION_1));

  toBeSavedFrame.lineMetadata(mLineMetadata);

  // Second filePath argument to add file name as extra information in XML
  saveMultipartRangeFrame(toBeSavedFrame, filePath, filePath);
}

}}
