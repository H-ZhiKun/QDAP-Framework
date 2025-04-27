// Copyright 2016-2020 SICK AG. All rights reserved.

#include "SaveBuffer.h"

#include "DatAndXmlFiles.h"
#include "../public/Exceptions.h"
#include "../public/GenIRanger.h"
#include "XmlElement.h"

#include <algorithm>
#include <iterator>

using namespace GenApi;

namespace {

const size_t STATUS_BIT_ENABLE = 30;
const size_t STATUS_BIT_ENCODER_A = 28;
const size_t STATUS_BIT_ENCODER_B = 27;
const size_t STATUS_BIT_OVERTRIG = 16;
const size_t STATUS_BIT_LINE_TRIGGER = 25;
const size_t STATUS_BIT_ENCODER_RESET = 24;


size_t bytesPerPixel(geniranger::PixelWidth pixelWidth)
{
  return static_cast<size_t>(pixelWidth) / 8;
}

size_t bytesPerLineInComponent(size_t aoiWidth,
                               geniranger::ConstComponentPtr component)
{
  return component ? bytesPerPixel(component->pixelWidth()) * aoiWidth : 0;
}

size_t bytesPerLineInRegion(geniranger::ConstRegionPtr region)
{
  size_t componentSize = 0;
  const size_t aoiWidth = region->aoiSize().x();
  componentSize += bytesPerLineInComponent(aoiWidth, region->range());
  componentSize += bytesPerLineInComponent(aoiWidth, region->reflectance());
  componentSize += bytesPerLineInComponent(aoiWidth, region->scatter());
  return componentSize;
}

std::string valueTypeFromPixelWidth(geniranger::PixelWidth pixelWidth)
{
  switch (pixelWidth)
  {
  case geniranger::PixelWidth::PW8: return "BYTE"; break;
  case geniranger::PixelWidth::PW16: return "WORD"; break;
  default: throw geniranger::GenIRangerException("Unexpected pixel format");
  }
}

void insertArbitraryXml(std::ostream& sstream,
                        std::string& endTag,
                        const std::string& arbitraryXml)
{
  // Seek backwards to just before the end tag
  sstream.seekp(-static_cast<int8_t>(endTag.size()), std::ostream::cur);
  sstream << arbitraryXml;
  // The previous write overwrote the end tag, so add it again
  sstream << "\n" << endTag;
}

template<typename T>
const geniranger::XmlElement parameter(const std::string& name, T value)
{
  return geniranger::xml("parameter").attribute("name", name).text(value);
}

/**
 * Returns a transformed value of the metadata at index, or zero if there is no
 * metadata.
 */
template<typename T>
uint32_t bitsFromMetadata(const std::vector<T>& data,
                          size_t index,
                          std::function<uint32_t(T)> transformer)
{
  return data.empty() ? 0 : transformer(data[index]);
}

/**
 * If available, returns the state of an IO shifted to the wanted position.
 * Otherwise, returns 0.
 */
uint32_t bitFromMetadata(std::vector<bool> data, size_t index, size_t bitNum)
{
  return bitsFromMetadata<bool>(data, index, [&](bool value) -> uint32_t {
    return (value & 0x01) << bitNum;
  });
}

/** Return value at index, unless the data is empty */
template<typename T>
T safeGet(const std::vector<T>& data, size_t index)
{
  return data.empty() ? 0 : data[index];
}


template<class SIZE>
void writeData(std::ostream& dataStream, const uint8_t* data, const SIZE size)
{
  dataStream.write(reinterpret_cast<const char*>(data),
                   static_cast<const size_t>(size));
}

void writeComponent(std::ostream& dataStream,
                    geniranger::ConstComponentPtr component)
{
  writeData(dataStream, component->data().data(), component->data().size());
}

void writeRegion(std::ostream& dataStream, geniranger::ConstRegionPtr region)
{
  if (region->range())
  {
    writeComponent(dataStream, region->range());
  }

  if (region->reflectance())
  {
    writeComponent(dataStream, region->reflectance());
  }
  if (region->scatter())
  {
    writeComponent(dataStream, region->scatter());
  }
}

void writeSingleMarkValue(std::ostream& dataStream,
                          const geniranger::SaveBuffer::MarkdataWord value)
{
  writeData(dataStream,
            reinterpret_cast<const uint8_t*>(&value),
            sizeof(geniranger::SaveBuffer::MarkdataWord));
}

/**
 * Write line mark data, a.k.a GigEVision line chunk data, to DAT-file. Each
 * line mark data consists of 5 values, each 4 bytes (32 bits) in size. They are
 * ordered in the following sequence for each line (profile) in data:
 * - Encoder value
 * - Status bits (zero-indexed from lsb):
 *   - Bit 16-23: Overtrig
 *   - Bit 27: Encoder B
 *   - Bit 28: Encoder A
 *   - Bit 30: Enable
 * - Sample time stamp
 * - Encoder pulse time stamp
 * - Camera scan ID
 */
void writeMarkData(std::ostream& dataStream, geniranger::LineMetadata metadata)
{
  const geniranger::SaveBuffer::MarkdataWord notUsed = 0;
  std::vector<size_t> sizes = {metadata.encoderValues.size(),
                               metadata.timestamps.size(),
                               metadata.overtriggerCounts.size(),
                               metadata.frameTriggerActive.size(),
                               metadata.lineTriggerActive.size(),
                               metadata.encoderResetActive.size(),
                               metadata.encoderA.size(),
                               metadata.encoderB.size()};
  std::vector<size_t> nonZeroSizes;

  // Collect only metadata that actually have any values
  std::copy_if(sizes.begin(),
               sizes.end(),
               std::back_inserter(nonZeroSizes),
               [](size_t i) { return i > 0; });

  if (nonZeroSizes.empty())
  {
    throw geniranger::SaveException("No metadata to write (size = 0)");
  }

  bool hasSameSize = std::equal(
    nonZeroSizes.begin() + 1, nonZeroSizes.end(), nonZeroSizes.begin());
  if (!hasSameSize)
  {
    std::stringstream str;
    str << "Inconsistent metadata length: "
        << "encoder: " << metadata.encoderValues.size() + ", "
        << "timestamp: " << metadata.timestamps.size() + ", "
        << "overtrig: " << metadata.overtriggerCounts.size() + ", "
        << "io1: " << metadata.overtriggerCounts.size() + ", "
        << "encoderA: " << metadata.overtriggerCounts.size() + ", "
        << "encoderB: " << metadata.overtriggerCounts.size() + ", ";
    throw geniranger::SaveException(str.str());
  }

  size_t someSize = nonZeroSizes[0];
  for (size_t i = 0; i < someSize; ++i)
  {
    writeSingleMarkValue(dataStream, safeGet(metadata.encoderValues, i));

    uint32_t statusBits = 0;
    statusBits |= bitsFromMetadata<uint8_t>(
      metadata.overtriggerCounts, i, [&](uint8_t value) {
        return value << STATUS_BIT_OVERTRIG;
      });
    statusBits |=
      bitFromMetadata(metadata.frameTriggerActive, i, STATUS_BIT_ENABLE);
    statusBits |=
      bitFromMetadata(metadata.lineTriggerActive, i, STATUS_BIT_LINE_TRIGGER);
    statusBits |=
      bitFromMetadata(metadata.encoderResetActive, i, STATUS_BIT_ENCODER_RESET);
    statusBits |= bitFromMetadata(metadata.encoderA, i, STATUS_BIT_ENCODER_A);
    statusBits |= bitFromMetadata(metadata.encoderB, i, STATUS_BIT_ENCODER_B);

    writeSingleMarkValue(dataStream, statusBits);
    // The file format supports only 32-bit timestamps. Keep the lowest bits
    // to keep the precision, and give up the range.
    writeSingleMarkValue(dataStream,
                         safeGet(metadata.timestamps, i) & 0xffffffff);
    writeSingleMarkValue(dataStream, notUsed);
    writeSingleMarkValue(dataStream, notUsed);
  }
}

}

namespace geniranger {

void writeFrame(std::ostream& dataStream, const Frame& frame)
{
  for (ConstRegionPtr region : frame.regions())
  {
    writeRegion(dataStream, region);
  }
  if (!frame.lineMetadata()->empty())
  {
    writeMarkData(dataStream, *frame.lineMetadata());
  }
}

XmlElement sensorRangeTraits(const int64_t bufferWidth,
                             const int64_t aoiHeight,
                             const int64_t aoiOffsetX,
                             const int64_t aoiOffsetY,
                             const RangeAxis rangeAxis)
{
  auto originZ = SaveBuffer::calculateOriginZ(aoiOffsetY, aoiHeight, rangeAxis);
  auto scaleZ = SaveBuffer::calculateScaleZ(rangeAxis);

  // From where on the sensor the data comes from can be calculated by
  // using the origin and scale factors below
  return xml("sensorrangetraits")
    // Value range of data in X-direction, 0-indexed
    .addChild(parameter("fov x0", 0))
    .addChild(parameter("fov x1", bufferWidth - 1))
    .addChild(parameter("fov x2", 0))
    .addChild(parameter("fov x3", bufferWidth - 1))
    // Value range of data in Z-direction (height), 0-indexed. Starts at 1
    // because 0 indicates missing data
    .addChild(parameter("fov z0", 1))
    .addChild(parameter("fov z1", 1))
    // 16 bits data with sub-pixel accuracy
    .addChild(parameter("fov z2", aoiHeight * 16 - 1))
    .addChild(parameter("fov z3", aoiHeight * 16 - 1))
    // Where on the sensor is the data extracted from
    .addChild(parameter("origin x", aoiOffsetX))
    .addChild(parameter("scale x", 1))
    .addChild(parameter("origin z", originZ))
    // Sub-pixel accuracy
    .addChild(parameter("scale z", scaleZ));
}

XmlElement sensorRangeTraits(const Region& region)
{
  return sensorRangeTraits(region.aoiSize().x(),
                           region.aoiSize().y(),
                           region.aoiOffset().x(),
                           region.aoiOffset().y(),
                           region.rangeAxis());
}

XmlElement subComponent(const std::string& name,
                        PixelWidth pixelWidth,
                        int64_t bufferWidth)
{
  return xml("subcomponent")
    .attribute("valuetype", valueTypeFromPixelWidth(pixelWidth))
    .attribute("name", name)
    .addChild(parameter("size", bufferWidth * bytesPerPixel(pixelWidth)))
    .addChild(parameter("width", bufferWidth));
}


XmlElement subComponentMarkData()
{
  return xml("subcomponent")
    .attribute("valuetype", "INT")
    .attribute("name", "Mark")
    .addChild(parameter("size", 20))
    .addChild(parameter("width", 5));
}

/**
 * Helper function to create a multipart frame from backwards compatible
 * parameters.
 */
Frame createMultiPartBuffer(const uint8_t* bufferRange16,
                            const uint8_t* bufferReflectance8,
                            const uint8_t* bufferScatter8,
                            const size_t bufferWidth,
                            const size_t bufferHeight,
                            const size_t aoiHeight,
                            const size_t aoiOffsetX,
                            const size_t aoiOffsetY,
                            const RangeAxis rangeAxis)
{
  size_t bufferSize = bufferWidth * bufferHeight;
  Region region;
  region.aoiOffset(aoiOffsetX, aoiOffsetY)
    .aoiSize(bufferWidth, aoiHeight)
    .rangeAxis(rangeAxis);

  assert(bufferRange16 != nullptr);
  region.createRange(bufferSize * 2, PixelWidth::PW16);
  region.range()->data().assign(bufferRange16, bufferRange16 + 2 * bufferSize);

  if (bufferReflectance8 != nullptr)
  {
    region.createReflectance(bufferSize, PixelWidth::PW8);
    region.reflectance()->data().assign(bufferReflectance8,
                                        bufferReflectance8 + bufferSize);
  }
  if (bufferScatter8 != nullptr)
  {
    region.createScatter(bufferSize, PixelWidth::PW8);
    region.scatter()->data().assign(bufferScatter8,
                                    bufferScatter8 + bufferSize);
  }
  return Frame::withRegion(region);
}

GENIRANGER_API void saveBuffer16(const uint8_t* buffer,
                                 const int64_t bufferWidth,
                                 const int64_t bufferHeight,
                                 const int64_t aoiHeight,
                                 const int64_t aoiOffsetX,
                                 const int64_t aoiOffsetY,
                                 const RangeAxis rangeAxis,
                                 const std::string& filePath,
                                 const std::string& arbitraryXml)
{
  saveMultipartRangeFrame(createMultiPartBuffer(buffer,
                                                nullptr,
                                                nullptr,
                                                bufferWidth,
                                                bufferHeight,
                                                aoiHeight,
                                                aoiOffsetX,
                                                aoiOffsetY,
                                                rangeAxis),
                          filePath,
                          arbitraryXml);
}

GENIRANGER_API void saveMultiPartBuffer(const uint8_t* bufferRange16,
                                        const uint8_t* bufferReflectance8,
                                        const uint8_t* bufferScatter8,
                                        const int64_t bufferWidth,
                                        const int64_t bufferHeight,
                                        const int64_t aoiHeight,
                                        const int64_t aoiOffsetX,
                                        const int64_t aoiOffsetY,
                                        const RangeAxis rangeAxis,
                                        const std::string& filePath,
                                        const std::string& arbitraryXml)
{
  saveMultipartRangeFrame(createMultiPartBuffer(bufferRange16,
                                                bufferReflectance8,
                                                bufferScatter8,
                                                bufferWidth,
                                                bufferHeight,
                                                aoiHeight,
                                                aoiOffsetX,
                                                aoiOffsetY,
                                                rangeAxis),
                          filePath,
                          arbitraryXml);
}


XmlElement createIConXmlSensorImage(const int64_t bufferWidth,
                                    const int64_t bufferHeight,
                                    const int64_t offsetX,
                                    const int64_t offsetY)
{
  return xml("icon_data_format")
    .attribute("valuetype", "IMAGE")
    .attribute("name", "IMAGE")
    .addChild(parameter("size", bufferWidth * bufferHeight))
    .addChild(parameter("version", 1))
    .addChild(parameter("layout", "SCAN"))
    .addChild(xml("component")
                .attribute("valuetype", "image")
                .attribute("name", "Ranger3Image")
                .addChild(parameter("size", bufferWidth))
                .addChild(parameter("height", bufferHeight))
                .addChild(xml("imagetraits")
                            .addChild(parameter("size x", bufferWidth))
                            .addChild(parameter("size y", bufferHeight))
                            .addChild(parameter("offset x", offsetX))
                            .addChild(parameter("offset y", offsetY)))
                .addChild(xml("subcomponent")
                            .attribute("valuetype", "BYTE")
                            .attribute("name", "Image")
                            .addChild(parameter("size", bufferWidth))
                            .addChild(parameter("width", bufferWidth))));
}

void saveSensorImage(const uint8_t* buffer,
                     const int64_t bufferWidth,
                     const int64_t bufferHeight,
                     const int64_t offsetX,
                     const int64_t offsetY,
                     std::ostream& dataStream,
                     std::ostream& xmlStream,
                     const std::string& arbitraryXml)
{
  XmlElement iconData =
    createIConXmlSensorImage(bufferWidth, bufferHeight, offsetX, offsetY);
  iconData.writeTo(xmlStream);

  writeData(dataStream, buffer, bufferWidth * bufferHeight);

  // Insert arbitrary xml just before the ending tag
  std::string endTag = "</icon_data_format>\n";
  insertArbitraryXml(xmlStream, endTag, arbitraryXml);
}

GENIRANGER_API void saveBuffer8(const uint8_t* buffer,
                                const int64_t bufferWidth,
                                const int64_t bufferHeight,
                                const int64_t offsetX,
                                const int64_t offsetY,
                                const std::string& filePath,
                                const std::string& arbitraryXml)
{
  DatAndXmlFiles files(filePath);
  saveSensorImage(buffer,
                  bufferWidth,
                  bufferHeight,
                  offsetX,
                  offsetY,
                  files.dataStream(),
                  files.xmlStream(),
                  arbitraryXml);
}

XmlElement createIConXml(const Frame& frame)
{
  const auto metadata = frame.lineMetadata();

  size_t totalBytesPerLine = 0;
  for (ConstRegionPtr region : frame.regions())
  {
    totalBytesPerLine += bytesPerLineInRegion(region);
  }
  if (!metadata->empty())
  {
    totalBytesPerLine += sizeof(SaveBuffer::MarkdataWord) * 5;
  }

  XmlElement root = xml("icon_data_format")
                      .attribute("valuetype", "")
                      .attribute("name", "")
                      .addChild(parameter("size", totalBytesPerLine))
                      .addChild(parameter("version", 1))
                      .addChild(parameter("layout", "SUBCOMPONENT"));

  for (ConstRegionPtr region : frame.regions())
  {
    const size_t aoiWidth = region->aoiSize().x();
    ConstComponentPtr range = region->range();
    ConstComponentPtr reflectance = region->reflectance();
    ConstComponentPtr scatter = region->scatter();
    // TODO: RNX-4506: This should not be hard coded, since the region might
    // contain something else
    XmlElement component = xml("component")
                             .attribute("valuetype", "Hi3D")
                             .attribute("name", "Ranger3Range")
                             .addChild(parameter("size", totalBytesPerLine))
                             .addChild(parameter("height", 1))
                             .addChild(sensorRangeTraits(*region));

    if (range)
    {
      component = component.addChild(
        subComponent("Range", range->pixelWidth(), aoiWidth));
    }
    if (reflectance)
    {
      component = component.addChild(
        subComponent("Intensity", reflectance->pixelWidth(), aoiWidth));
    }
    if (scatter)
    {
      component = component.addChild(
        subComponent("Scatter", scatter->pixelWidth(), aoiWidth));
    }
    if (!metadata->empty())
    {
      // TODO: RNX-4506: should this really be per subcomponent?
      component = component.addChild(subComponentMarkData());
    }

    root = root.addChild(component);
  }
  return root;
}

void saveMultipartRangeFrame(const Frame& frame,
                             std::ostream& dataStream,
                             std::ostream& xmlStream,
                             const std::string& arbitraryXml)
{
  if (frame.regions().empty())
  {
    throw SaveException("Frame needs at least one region.");
  }

  if (frame.regions().size() > 1)
  {
    throw SaveException("Saving multiple regions is not supported yet");
  }

  for (ConstRegionPtr region : frame.regions())
  {
    ConstComponentPtr range = region->range();
    ConstComponentPtr reflectance = region->reflectance();
    if (!range)
    {
      throw SaveException("Range component must exist, when saving.");
    }
    ConstComponentPtr scatter = region->scatter();
    if (range && range->pixelWidth() != PixelWidth::PW16)
    {
      throw SaveException(
        "Range component must have pixel width 16, when saving.");
    }
    if (scatter
        && !(scatter->pixelWidth() == PixelWidth::PW8
             || scatter->pixelWidth() == PixelWidth::PW16))
    {
      throw SaveException(
        "Scatter component must have pixel width 8 or 16, when saving.");
    }
    if (reflectance && reflectance->pixelWidth() != PixelWidth::PW8)
    {
      throw SaveException(
        "Reflectance component must have pixel width 8, when saving.");
    }
  }

  writeFrame(dataStream, frame);
  createIConXml(frame).writeTo(xmlStream);

  // Insert arbitrary xml just before the ending tag
  std::string endTag = "</icon_data_format>\n";
  insertArbitraryXml(xmlStream, endTag, arbitraryXml);
}

GENIRANGER_API void saveMultipartRangeFrame(const Frame& frame,
                                            const std::string& filePath,
                                            const std::string& arbitraryXml)
{
  DatAndXmlFiles files(filePath);
  saveMultipartRangeFrame(
    frame, files.dataStream(), files.xmlStream(), arbitraryXml);
}

/**
 * Save caliibrated Range Data.
 * Add by VisionLab, Shanghai China.
 */
GENIRANGER_API void saveBufferFloat(const std::vector<void*> & buffers,
									const int64_t & bufferWidth,
									const int64_t & bufferHeight,
									//const int64_t & aoiHeight,
									//const int64_t & aoiOffsetX,
									const float & Lx,
									const float & Lr,
									const float & Ux,
									const float & Ur,
									const std::string & filePath)
{
	if (buffers.empty() || buffers[0] == nullptr) return;

	// write data
	const size_t aoiWidth = bufferWidth;

	std::ofstream iconFile(filePath + ".dat", std::ios::binary);
	iconFile.write(reinterpret_cast<const char*>(buffers[0]),
		static_cast<const size_t>(sizeof(float) * aoiWidth * bufferHeight));

	size_t totalSize = sizeof(float) * aoiWidth;

	if (buffers.size() >= 2 && buffers[1] != nullptr)
	{
		iconFile.write(reinterpret_cast<const char*>(buffers[1]),
			static_cast<const size_t>(sizeof(uint8_t) * aoiWidth * bufferHeight));

		totalSize += sizeof(uint8_t) * aoiWidth;
	}
	if (buffers.size() >= 3 && buffers[2] != nullptr)
	{
		iconFile.write(reinterpret_cast<const char*>(buffers[2]),
			static_cast<const size_t>(sizeof(uint8_t) * aoiWidth * bufferHeight));

		totalSize += sizeof(uint8_t) * aoiWidth;
	}

	// write xml
	std::ofstream xmlFile(filePath + ".xml");
	xmlFile << "<icon_data_format valuetype=\"\" name=\"\">\n";
	xmlFile << "  <parameter name=\"size\">" << std::to_string(totalSize) << "</parameter>\n";
	xmlFile << "  <parameter name=\"version\">1</parameter>\n";
	xmlFile << "  <parameter name=\"layout\">SUBCOMPONENT</parameter>\n";
	xmlFile << "  <component valuetype=\"Hi3D\" name=\"Ranger3Range\">\n";
	xmlFile << "    <parameter name=\"size\">" << std::to_string(totalSize) << "</parameter>\n";
	xmlFile << "    <parameter name=\"height\">1</parameter>\n";
	xmlFile << "    <worldrangetraits>\n";
	xmlFile << "      <parameter name=\"lower bound x\">" << std::to_string(Lx) << "</parameter>\n";
	xmlFile << "      <parameter name=\"lower bound R\">" << std::to_string(Lr) << "</parameter>\n";
	xmlFile << "      <parameter name=\"upper bound x\">" << std::to_string(Ux) << "</parameter>\n";
	xmlFile << "      <parameter name=\"upper bound R\">" << std::to_string(Ur) << "</parameter>\n";
	xmlFile << "      <parameter name=\"coordinate unit\"></parameter>\n";
	xmlFile << "    </worldrangetraits>\n";
	xmlFile << "    <subcomponent valuetype=\"FLOAT\" name=\"Range\">\n";
	xmlFile << "      <parameter name=\"size\">" << std::to_string(aoiWidth * sizeof(float)) << "</parameter>\n";
	xmlFile << "      <parameter name=\"width\">" << std::to_string(aoiWidth) << "</parameter>\n";
	xmlFile << "    </subcomponent>\n";
	if (buffers.size() >= 2 && buffers[1] != nullptr)	{
	xmlFile << "    <subcomponent valuetype=\"BYTE\" name=\"Intensity\">\n";
	xmlFile << "      <parameter name=\"size\">" << std::to_string(aoiWidth) << "</parameter>\n";
	xmlFile << "      <parameter name=\"width\">" << std::to_string(aoiWidth) << "</parameter>\n";
	xmlFile << "    </subcomponent>\n";
	}
	if (buffers.size() >= 3 && buffers[2] != nullptr)	{
	xmlFile << "    <subcomponent valuetype=\"BYTE\" name=\"Scatter\"\n>";
	xmlFile << "      <parameter name=\"size\">" << std::to_string(aoiWidth) << "</parameter>\n";
	xmlFile << "      <parameter name=\"width\">" << std::to_string(aoiWidth) << "</parameter>\n";
	xmlFile << "    </subcomponent>\n";
	}
	xmlFile << "  </component>\n";
	xmlFile << "</icon_data_format>";
	xmlFile.close();
	
	return;
}

}
