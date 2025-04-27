// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include <map>
#include <memory>
#include <stdint.h>
#include <vector>
#include <stdexcept>

namespace geniranger {
/** A positive (1st quadrant) point in cartesian 2-dimensional plane. */
class Size2D
{
public:
  Size2D(size_t x, size_t y)
    : mX(x)
    , mY(y)
  {
  }

  size_t x() const { return mX; }
  size_t y() const { return mY; }

  bool operator==(const Size2D& other) const
  {
    return mX == other.x() && mY == other.y();
  };

private:
  size_t mX;
  size_t mY;
};

/** Width in bits for a pixel. */
enum class PixelWidth
{
  PW8 = 8,
  PW12 = 12,
  PW16 = 16
};

/** Range axis direction. */
enum class RangeAxis
{
  STANDARD = 0,
  REVERSE,
};

inline RangeAxis rangeAxisFromString(const std::string& genapiName)
{
  if (genapiName == "Standard")
  {
    return RangeAxis::STANDARD;
  }
  else if (genapiName == "Reverse")
  {
    return RangeAxis::REVERSE;
  }
  throw std::invalid_argument(std::string("Unknown range axis: ") + genapiName);
}

inline std::string toString(RangeAxis rangeAxis)
{
  switch (rangeAxis)
  {
  case RangeAxis::STANDARD: return "Standard";
  case RangeAxis::REVERSE: return "Reverse";
  }
  throw std::invalid_argument("Unknown range axis");
}

/** Container for raw payload data (bytes). */
typedef std::vector<uint8_t> DataVector;

/** Data payload of specific component type. */
class Component
{
public:
  Component(size_t sizeInBytes, PixelWidth pw)
    : mData(sizeInBytes)
    , mPixelWidth(pw)
  {
  }

  Component(const DataVector& data, PixelWidth pw)
    : mData(data)
    , mPixelWidth(pw)
  {
  }

  Component& data(DataVector& data)
  {
    mData = data;
    return *this;
  }
  DataVector& data() { return mData; }
  const DataVector& data() const { return mData; }

  PixelWidth pixelWidth() const { return mPixelWidth; }
  Component& pixelWidth(PixelWidth pw)
  {
    mPixelWidth = pw;
    return *this;
  }

private:
  DataVector mData;
  PixelWidth mPixelWidth;
};

/** Container for metadata for a series of lines. */
struct LineMetadata
{
public:
  bool empty() const
  {
    return encoderValues.empty() && timestamps.empty()
           && overtriggerCounts.empty() && frameTriggerActive.empty()
           && lineTriggerActive.empty() && encoderResetActive.empty()
           && encoderA.empty() && encoderB.empty();
  }

public:
  typedef std::vector<uint32_t> EncoderValues;
  typedef std::vector<int64_t> Timestamps;
  typedef std::vector<uint8_t> OverTriggers;
  typedef std::vector<bool> InputSignals;

  EncoderValues encoderValues;
  Timestamps timestamps;
  OverTriggers overtriggerCounts;
  InputSignals frameTriggerActive;
  InputSignals lineTriggerActive;
  InputSignals encoderResetActive;
  InputSignals encoderA;
  InputSignals encoderB;
};



/** Pointer to Component. */
typedef std::shared_ptr<Component> ComponentPtr;
typedef std::shared_ptr<const Component> ConstComponentPtr;

/**
 * A range data region containing some AOI information and some data Component.
 */
class Region
{
public:
  Size2D aoiOffset() const { return mAoiOffset; }
  Region& aoiOffset(Size2D offset)
  {
    mAoiOffset = offset;
    return *this;
  }
  Region& aoiOffset(size_t x, size_t y) { return aoiOffset(Size2D(x, y)); }

  Size2D aoiSize() const { return mAoiSize; }
  Region& aoiSize(Size2D size)
  {
    mAoiSize = size;
    return *this;
  }
  Region& aoiSize(size_t x, size_t y) { return aoiSize(Size2D(x, y)); }

  RangeAxis rangeAxis() const { return mRangeAxis; }
  Region& rangeAxis(RangeAxis axis)
  {
    mRangeAxis = axis;
    return *this;
  }

  ComponentPtr range() { return mRange; }
  ConstComponentPtr range() const { return mRange; }
  ComponentPtr reflectance() { return mReflectance; }
  ConstComponentPtr reflectance() const { return mReflectance; }
  ComponentPtr scatter() { return mScatter; }
  ConstComponentPtr scatter() const { return mScatter; }

  Region& createRange(size_t bytes, PixelWidth pw)
  {
    mRange = std::make_shared<Component>(bytes, pw);
    return *this;
  }

  Region& createRange(const DataVector& data, PixelWidth pw)
  {
    mRange = std::make_shared<Component>(data, pw);
    return *this;
  }

  Region& createReflectance(size_t bytes, PixelWidth pw)
  {
    mReflectance = std::make_shared<Component>(bytes, pw);
    return *this;
  }

  Region& createReflectance(const DataVector& data, PixelWidth pw)
  {
    mReflectance = std::make_shared<Component>(data, pw);
    return *this;
  }

  Region& createScatter(size_t bytes, PixelWidth pw)
  {
    mScatter = std::make_shared<Component>(bytes, pw);
    return *this;
  }

  Region& createScatter(const DataVector& data, PixelWidth pw)
  {
    mScatter = std::make_shared<Component>(data, pw);
    return *this;
  }

  bool operator==(const Region& other) const
  {
    return mAoiSize == other.mAoiSize && mAoiOffset == other.mAoiOffset
           && mRangeAxis == other.mRangeAxis && mRange == other.mRange
           && mReflectance == other.mReflectance && mScatter == other.mScatter;
  }

private:
  Size2D mAoiSize{0, 0};
  Size2D mAoiOffset{0, 0};
  RangeAxis mRangeAxis{RangeAxis::REVERSE};

  ComponentPtr mRange;
  ComponentPtr mReflectance;
  ComponentPtr mScatter;
};

/** Pointer to Region. */
typedef std::shared_ptr<Region> RegionPtr;
typedef std::vector<RegionPtr> RegionPtrs;
typedef std::shared_ptr<const Region> ConstRegionPtr;
typedef std::vector<ConstRegionPtr> ConstRegionPtrs;

/** Pointer to LineMetadata. */
typedef std::shared_ptr<LineMetadata> LineMetadataPtr;

/**
 * A data stream frame consists of one or several regions, and meta data.
 *
 * This class is deprecated and should no longer be used. Prefer using \ref
 * geniranger::frame::IFrame instead.
 *
 * \see \ref geniranger::FrameGrabber
 */
class Frame
{
public:
  Frame()
    : mLineMetadata(std::make_shared<LineMetadata>())
  {
  }

  RegionPtr createRegion()
  {
    RegionPtr region = std::make_shared<Region>();
    mRegions.push_back(region);
    return region;
  }

  RegionPtrs& regions() { return mRegions; }
  /** \return a const copy with const pointers to avoid changing the regions */
  const ConstRegionPtrs regions() const
  {
    ConstRegionPtrs result;
    for (auto& region : mRegions)
    {
      result.push_back(region);
    }
    return result;
  }

  const std::shared_ptr<const LineMetadata> lineMetadata() const
  {
    return mLineMetadata;
  }

  LineMetadataPtr lineMetadata() { return mLineMetadata; }

  Frame& lineMetadata(LineMetadataPtr lineMetadata)
  {
    mLineMetadata = lineMetadata;
    return *this;
  }

  static const Frame withRegion(const Region& region)
  {
    Frame frame;
    frame.mRegions.push_back(RegionPtr(new Region(region)));
    return frame;
  }

private:
  RegionPtrs mRegions;
  LineMetadataPtr mLineMetadata;
};
}
