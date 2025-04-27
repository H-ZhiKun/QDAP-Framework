// Copyright 2019-2020 SICK AG. All rights reserved.
#include "../../public/frame/Frame.h"
#include "../../public/ChunkAdapter.h"
#include "../../public/FastMetadataExtractor.h"
#include "../../public/FrameGrabber.h"
#include "../../public/gentlcpp/BufferPart.h"
#include "../EnumSelectorEntries.h"
#include "GenTlBackedFrame.h"
#include <functional>

namespace {

/** Extracts the Ranger3 specific line metadata from chunks in a buffer */
std::shared_ptr<geniranger::LineMetadata>
extractMetadata(geniranger::NodeMap device,
                std::shared_ptr<gentlcpp::Buffer> buffer)
{
  if (!buffer->containsChunkData())
  {
    // Return an empty metadata object
    return std::make_shared<geniranger::LineMetadata>();
  }

  geniranger::ChunkAdapter chunkAdapter(device, buffer);

  geniranger::FastMetadataExtractor fastExtractor(device);
  auto result = fastExtractor.extract();

  auto metadata = std::make_shared<geniranger::LineMetadata>();
  for (Ranger3LineMetadata line : result)
  {
    metadata->encoderA.push_back(line.encoderA);
    metadata->encoderB.push_back(line.encoderB);
    metadata->encoderValues.push_back(line.encoderValue);
    metadata->encoderResetActive.push_back(line.encoderResetActive);
    metadata->timestamps.push_back(line.timestamp);
    metadata->frameTriggerActive.push_back(line.frameTriggerActive);
    metadata->lineTriggerActive.push_back(line.lineTriggerActive);
    metadata->overtriggerCounts.push_back(line.overtriggerCount);
  }
  return metadata;
}

geniranger::frame::DeviceScanType getDeviceScanType(geniranger::NodeMap nodeMap)
{
  return (nodeMap.getEnum("DeviceScanType") == "Areascan")
           ? geniranger::frame::DeviceScanType::AREASCAN
           : geniranger::frame::DeviceScanType::LINESCAN_3D;
}

bool regionExists(geniranger::NodeMap nodeMap, const std::string& region)
{
  GenApi::CEnumerationPtr selector = nodeMap.get()._GetNode("RegionSelector");
  GenApi::CEnumEntryPtr entry = selector->GetEntryByName(region.c_str());
  if (entry == nullptr)
  {
    return false;
  }

  GenApi::EAccessMode accessMode = entry->GetAccessMode();
  return accessMode != GenApi::EAccessMode::NA
         && accessMode != GenApi::EAccessMode::NI;
}

bool isRangeAxisAvailable(geniranger::NodeMap nodeMap,
                          const std::string& extraction)
{
  nodeMap.setEnum("Scan3dExtractionSelector", extraction);
  GenApi::EAccessMode access = nodeMap.getAccessMode("RangeAxis");
  return access == GenApi::EAccessMode::RO || access == GenApi::EAccessMode::RW;
}

geniranger::RangeAxis getRangeAxis(geniranger::NodeMap nodeMap,
                                   const std::string& extraction)
{
  nodeMap.setEnum("Scan3dExtractionSelector", extraction);
  return geniranger::rangeAxisFromString(nodeMap.getEnum("RangeAxis"));
}

void fillAoi(geniranger::NodeMap nodeMap,
             geniranger::frame::RegionConfigurationInfo& info)
{
  info.aoi.height = static_cast<uint16_t>(nodeMap.getInt("Height"));
  info.aoi.width = static_cast<uint16_t>(nodeMap.getInt("Width"));
  info.aoi.offsetX = static_cast<uint16_t>(nodeMap.getInt("OffsetX"));
  info.aoi.offsetY = static_cast<uint16_t>(nodeMap.getInt("OffsetY"));
}

}

namespace geniranger { namespace frame {

/**
 * When the BorrowedFrame is destructed, the underlying buffer is re-queued to
 * be reused by the acquisition engine.
 */
class BorrowedFrame : public GenTlBackedFrame
{
public:
  BorrowedFrame(std::shared_ptr<gentlcpp::Buffer> buffer,
                EnumSelectorEntries& regionLookup,
                EnumSelectorEntries& componentLookup,
                std::shared_ptr<LineMetadata> lineMetadata,
                const FrameConfigurationInfo& frameInfo,
                std::function<void()> onDestruct)
    : GenTlBackedFrame(buffer,
                       regionLookup,
                       componentLookup,
                       lineMetadata,
                       frameInfo)
    , mOnDestruct(onDestruct)
  {
  }

  ~BorrowedFrame() { mOnDestruct(); }

private:
  std::function<void()> mOnDestruct;
};


std::shared_ptr<IFrame>
fromBufferInternal(std::shared_ptr<gentlcpp::Buffer> buffer,
                   NodeMap deviceNodeMap,
                   const FrameConfigurationInfo& frameInfo,
                   std::function<void()> onDestruct)
{
  EnumSelectorEntries regionLookup(deviceNodeMap, "RegionSelector");
  EnumSelectorEntries componentLookup(deviceNodeMap, "ComponentSelector");

  return std::make_shared<BorrowedFrame>(buffer,
                                         regionLookup,
                                         componentLookup,
                                         extractMetadata(deviceNodeMap, buffer),
                                         frameInfo,
                                         onDestruct);
}

std::shared_ptr<IFrame> fromBuffer(std::shared_ptr<gentlcpp::Buffer> buffer,
                                   NodeMap deviceNodeMap,
                                   const FrameConfigurationInfo& frameInfo,
                                   std::shared_ptr<FrameGrabber> grabber)
{
  return fromBufferInternal(
    buffer, deviceNodeMap, frameInfo, [=]() { grabber->queueBuffer(buffer); });
}

std::shared_ptr<IFrame>
fromBuffer(std::shared_ptr<gentlcpp::Buffer> buffer,
           NodeMap deviceNodeMap,
           const FrameConfigurationInfo& frameInfo,
           std::shared_ptr<gentlcpp::DataStream> dataStream)
{
  return fromBufferInternal(buffer, deviceNodeMap, frameInfo, [=]() {
    dataStream->queueBuffer(buffer);
  });
}

FrameConfigurationInfo fromDeviceConfiguration(NodeMap deviceNodeMap)
{
  FrameConfigurationInfo frameInfo;
  frameInfo.deviceScanType = getDeviceScanType(deviceNodeMap);

  // Fetch info about 2D data extraction
  RegionConfigurationInfo& region0Info = frameInfo.regions[RegionId::REGION_0];
  deviceNodeMap.setEnum("RegionSelector", "Region0");
  fillAoi(deviceNodeMap, region0Info);
  // TODO RNX-5208: Dummy value for Region0 since not valid for 2D
  region0Info.rangeAxis = RangeAxis::STANDARD;

  RangeAxis region1RangeAxis = getRangeAxis(deviceNodeMap, "Scan3dExtraction1");

  // Currently supporting up to 5 regions in 3D
  for (int i = 1; i <= 5; ++i)
  {
    std::string sensorRegion = std::string("Region") + std::to_string(i);

    if (regionExists(deviceNodeMap, sensorRegion))
    {
      RegionId regionId = static_cast<RegionId>(
        static_cast<int>(RegionId::SCAN_3D_EXTRACTION_1) + (i - 1));
      RegionConfigurationInfo& regionInfo = frameInfo.regions[regionId];

      // Fetch range axis
      std::string extraction =
        std::string("Scan3dExtraction") + std::to_string(i);
      if (isRangeAxisAvailable(deviceNodeMap, extraction))
      {
        regionInfo.rangeAxis = getRangeAxis(deviceNodeMap, extraction);
      }
      else
      {
        // TODO RNX-5208: Some regions don't have an explicit range axis, we use
        // the value from region 1 as a dummy value.
        regionInfo.rangeAxis = region1RangeAxis;
      }

      // Fetch info where on the sensor the 3D data is extracted
      deviceNodeMap.setEnum("RegionSelector", sensorRegion.c_str());
      fillAoi(deviceNodeMap, regionInfo);
    }
  }

  return frameInfo;
}

}}
