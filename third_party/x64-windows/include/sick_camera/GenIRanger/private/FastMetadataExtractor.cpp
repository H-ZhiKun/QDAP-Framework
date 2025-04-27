// Copyright 2018-2020 SICK AG. All rights reserved.

// This header must be included before windows.h, otherwise winsock 1 will also
// be included, which would lead to conflicting defines.
// This declares the functions ntohl and ntohll.
#include <winsock2.h>

#include "../public/FastMetadataExtractor.h"

#include <GenApi/IChunkPort.h>
#include <GenApi/IRegister.h>

namespace {

/**
 * Gets the backing register of an ordinary feature. This assumes the feature is
 * connected to a register via <pValue>.
 */
GenApi::CRegisterPtr getRegister(GenApi::IValue& feature)
{
  GenApi::NodeList_t children;
  feature.GetNode()->GetChildren(children,
                                 GenApi::ELinkType::ctReadingChildren);

  assert(children.size() == 1);

  GenApi::CRegisterPtr reg(children[0]);
  // Verify that the child is an actual register
  assert(reg.IsValid());
  return reg;
}

size_t getChunkPortLength(GenApi::CChunkPortPtr chunkPort)
{
  uint64_t length;
  chunkPort->Read(&length, CHUNK_LENGTH_REGISTER, CHUNK_LENGTH_REGISTER_LEN);
  return static_cast<size_t>(length);
}

}

namespace geniranger {

ChunkScanLineValueAccessor::ChunkScanLineValueAccessor(
  GenApi::CIntegerPtr chunkScanLineSelector,
  GenApi::CIntegerPtr feature)
{
  GenApi::CRegisterPtr registerNode = getRegister(*feature);
  chunkScanLineSelector->SetValue(0);
  mBase = registerNode->GetAddress();
  chunkScanLineSelector->SetValue(1);
  mIncrement = registerNode->GetAddress() - mBase;
  mRegisterSize = registerNode->GetLength();
  assert(mRegisterSize == 4 || mRegisterSize == 8);

  GenICam::gcstring valueStr;
  GenICam::gcstring attributeStr;
  bool hasLsb =
    registerNode->GetNode()->GetProperty("LSB", valueStr, attributeStr);
  mLsb = hasLsb ? atoi(valueStr.c_str()) : (mRegisterSize == 4 ? 31 : 63);

  bool hasMsb =
    registerNode->GetNode()->GetProperty("MSB", valueStr, attributeStr);
  size_t msb = hasMsb ? atoi(valueStr.c_str()) : 0;
  mMask = mRegisterSize == 4 ? createBitMask32(mLsb, msb)
                             : createBitMask64(mLsb, msb);
}

uint64_t ChunkScanLineValueAccessor::getValue64(const uint8_t* buffer,
                                                size_t lineIndex)
{
  assert(mRegisterSize == 8);
  return getRange64(ntohll(*reinterpret_cast<const int64_t*>(
    &buffer[mBase + mIncrement * lineIndex])));
}

uint32_t ChunkScanLineValueAccessor::getValue32(const uint8_t* buffer,
                                                size_t lineIndex)
{
  assert(mRegisterSize == 4);
  return getRange32(
    static_cast<uint32_t>(ntohl(*reinterpret_cast<const uint32_t*>(
      &buffer[mBase + mIncrement * lineIndex]))));
}

FastMetadataExtractor::FastMetadataExtractor(NodeMap device)
  : mChunkScanLineSelector(device.get()._GetNode("ChunkScanLineSelector"))
  , mTimestamp(mChunkScanLineSelector, device.get()._GetNode("ChunkTimestamp"))
  , mEncoderValue(mChunkScanLineSelector,
                  device.get()._GetNode("ChunkEncoderValue"))
  , mOvertriggerCount(mChunkScanLineSelector,
                      device.get()._GetNode("ChunkOvertriggerCount"))
  , mFrameTriggerActive(mChunkScanLineSelector,
                        device.get()._GetNode("ChunkFrameTriggerActive"))
  , mLineTriggerActive(mChunkScanLineSelector,
                       device.get()._GetNode("ChunkLineTriggerActive"))
  , mEncoderResetActive(mChunkScanLineSelector,
                        device.get()._GetNode("ChunkEncoderResetActive"))
  , mEncoderA(mChunkScanLineSelector, device.get()._GetNode("ChunkEncoderA"))
  , mEncoderB(mChunkScanLineSelector, device.get()._GetNode("ChunkEncoderB"))
  , mChunkPort(device.get()._GetNode("DeviceChunkPort"))
{
}

std::vector<Ranger3LineMetadata> FastMetadataExtractor::extract()
{
  std::vector<uint8_t> buffer(getChunkPortLength(mChunkPort), 0);
  mChunkPort->Read(buffer.data(), 0, buffer.size());

  std::vector<Ranger3LineMetadata> metadata;
  const auto lineMin = mChunkScanLineSelector->GetMin();
  const auto lineMax = mChunkScanLineSelector->GetMax();

  // Pre-allocate the memory and access directly by index in the loop. This
  // avoids calling push_back, since that would be the most expensive operation
  // in debug builds
  metadata.resize(lineMax - lineMin + 1);

  for (int64_t i = lineMin; i <= lineMax; ++i)
  {
    size_t index = i - lineMin;
    Ranger3LineMetadata& lineMetadata = metadata[index];
    lineMetadata.timestamp = mTimestamp.getValue64(buffer.data(), i);
    lineMetadata.encoderValue =
      static_cast<int32_t>(mEncoderValue.getValue32(buffer.data(), i));
    lineMetadata.overtriggerCount =
      static_cast<uint8_t>(mOvertriggerCount.getValue32(buffer.data(), i));
    lineMetadata.frameTriggerActive =
      mFrameTriggerActive.getValue32(buffer.data(), i) ? true : false;
    lineMetadata.lineTriggerActive =
      mLineTriggerActive.getValue32(buffer.data(), i) ? true : false;
    lineMetadata.encoderResetActive =
      mEncoderResetActive.getValue32(buffer.data(), i) ? true : false;
    lineMetadata.encoderA =
      mEncoderA.getValue32(buffer.data(), i) ? true : false;
    lineMetadata.encoderB =
      mEncoderB.getValue32(buffer.data(), i) ? true : false;
  }
  return metadata;
}

}
