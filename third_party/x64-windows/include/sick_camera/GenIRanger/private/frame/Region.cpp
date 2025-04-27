// Copyright 2019-2020 SICK AG. All rights reserved.
#include "../../public/frame/Region.h"
#include "../GenIUtil.h"
#include "../../public/frame/Frame.h"
#include "../../public/gentlcpp/BufferPart.h"

#include <sstream>

namespace geniranger { namespace frame {

RegionId Region::getId() const
{
  return mRegionId;
};

ConstComponentList Region::getComponents() const
{
  return geniutil::constPointerVector(mComponents);
}

bool Region::hasComponent(ComponentId componentId) const
{
  return find(componentId) != nullptr;
}

std::shared_ptr<const Component>
Region::getComponent(ComponentId componentId) const
{
  std::shared_ptr<Component> result = find(componentId);
  if (result == nullptr)
  {
    std::ostringstream ss;
    ss << "No component with id: " << static_cast<uint64_t>(componentId);
    std::string message = ss.str();
    throw std::invalid_argument(message);
  }
  return result;
}

Aoi Region::getAoi() const
{
  return mAoi;
}

RangeAxis Region::getRangeAxis() const
{
  return mRangeAxis;
}

Region::Region(RegionId regionId, const RegionConfigurationInfo& regionInfo)
  : mRegionId(regionId)
  , mAoi(regionInfo.aoi)
  , mRangeAxis(regionInfo.rangeAxis)
{
  // Empty
}

std::shared_ptr<Component> Region::find(ComponentId componentId) const
{
  for (auto component : mComponents)
  {
    if (component->getId() == componentId)
    {
      return component;
    }
  }
  return std::shared_ptr<Component>();
}

std::shared_ptr<Component> Region::findOrAdd(ComponentId componentId,
                                             PixelFormat pixelFormat,
                                             uint16_t width,
                                             uint16_t height,
                                             uint8_t* data,
                                             size_t dataSize)
{
  auto component = find(componentId);
  if (!component)
  {
    mComponents.push_back(std::shared_ptr<Component>(
      new Component(componentId, pixelFormat, width, height, data, dataSize)));
    component = mComponents.back();
  }
  return component;
}

std::ostream& operator<<(std::ostream& s, const Region& region)
{
  s << "Region ID: " << toString(region.getId()) << std::endl;
  s << "AOI Width: " << region.getAoi().width << std::endl;
  s << "AOI Height: " << region.getAoi().height << std::endl;
  s << "AOI Offset X: " << region.getAoi().offsetX << std::endl;
  s << "AOI Offset Y: " << region.getAoi().offsetY << std::endl;
  s << "Range Axis: " << toString(region.getRangeAxis()) << std::endl;
  for (auto component : region.getComponents())
  {
    s << *component;
  }
  return s;
}

}}
