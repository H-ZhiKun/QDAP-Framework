// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include "NodeMap.h"
#include "Ranger3LineMetadata.h"

namespace geniranger {

/**
 * Helper class to access metadata values without reading via the slow GenApi
 * interface.
 */
class ChunkScanLineValueAccessor
{
public:
  ChunkScanLineValueAccessor(GenApi::CIntegerPtr chunkScanLineSelector,
                             GenApi::CIntegerPtr node);

  uint64_t getValue64(const uint8_t* buffer, size_t lineIndex);
  uint32_t getValue32(const uint8_t* buffer, size_t lineIndex);

private:
  inline static uint64_t createBitMask64(size_t lsb, size_t msb)
  {
    assert(lsb < 64);
    assert(msb <= lsb);
    return (0xffffffffffffffff >> (63 - lsb + msb)) << (63 - lsb);
  }

  inline static uint32_t createBitMask32(size_t lsb, size_t msb)
  {
    assert(lsb < 32);
    assert(msb <= lsb);
    return (0xffffffff >> (31 - lsb + msb)) << (31 - lsb);
  }

  inline uint64_t getRange64(uint64_t number)
  {
    return (mMask & number) >> (63 - mLsb);
  }

  inline uint32_t getRange32(uint32_t number)
  {
    return (static_cast<uint32_t>(mMask) & number) >> (31 - mLsb);
  }

private:
  size_t mBase;
  size_t mIncrement;
  size_t mRegisterSize;
  uint64_t mMask;
  size_t mLsb;
};

/**
 * Extracts metadata from chunk features given a NodeMap.
 *
 * This implementation doesn't use the GenApi standard way to iterate through
 * all the scan lines, by setting the ChunkScanLineSelector for every line.
 *
 * Instead this class will calculate the base address and offset for every
 * metadata feature, and assume that the offset is constant between every line.
 * This is fairly safe assumption for these features since they are selected by
 * a single selector.
 *
 * The registers will then be accessed directly via the ChunkPort, thereby
 * skipping the node map, selectors and swissknife calculations.
 */
class FastMetadataExtractor
{
public:
  FastMetadataExtractor(NodeMap device);
  std::vector<Ranger3LineMetadata> extract();

private:
  GenApi::CIntegerPtr mChunkScanLineSelector;
  ChunkScanLineValueAccessor mTimestamp;
  ChunkScanLineValueAccessor mEncoderValue;
  ChunkScanLineValueAccessor mOvertriggerCount;
  ChunkScanLineValueAccessor mFrameTriggerActive;
  ChunkScanLineValueAccessor mLineTriggerActive;
  ChunkScanLineValueAccessor mEncoderResetActive;
  ChunkScanLineValueAccessor mEncoderA;
  ChunkScanLineValueAccessor mEncoderB;
  GenApi::CChunkPortPtr mChunkPort;
};

}
