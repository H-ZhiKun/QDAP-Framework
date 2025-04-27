// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include "Component.h"
#include "../../public/StreamData.h"
#include "../../public/frame/Aoi.h"

namespace geniranger { namespace frame {

struct RegionConfigurationInfo;

/**
 * Abstraction of a region within a \ref IFrame. Each Region contains \ref
 * Component%s.
 */
class Region
{
public:
  /** \return the \ref RegionId of the region */
  RegionId getId() const;

  /** \return the \ref Component%s within the region */
  ConstComponentList getComponents() const;

  /**
   * \return true if the region contains a \ref Component with the given \ref
   *         ComponentId
   */
  bool hasComponent(ComponentId componentId) const;

  /**
   * \return the object for a specific ComponentId
   * \throw std::invalid_argument if the ComponentId is not available
   */
  std::shared_ptr<const Component> getComponent(ComponentId componentId) const;

  /** \return the area of interest on the sensor for the region */
  Aoi getAoi() const;

  /** \return the direction of the range axis for the region */
  RangeAxis getRangeAxis() const;

private:
  friend class GenTlBackedFrame;

  Region(RegionId regionId, const RegionConfigurationInfo& regionInfo);
  std::shared_ptr<Component> find(ComponentId componentId) const;
  std::shared_ptr<Component> findOrAdd(ComponentId componentId,
                                       PixelFormat pixelFormat,
                                       uint16_t width,
                                       uint16_t height,
                                       uint8_t* data,
                                       size_t dataSize);

private:
  RegionId mRegionId;
  ComponentList mComponents;
  Aoi mAoi;
  RangeAxis mRangeAxis;
};

typedef std::vector<std::shared_ptr<Region>> RegionList;
typedef std::vector<std::shared_ptr<const Region>> ConstRegionList;

/** Logs information about a \ref Region and the \ref Component%s in it. */
std::ostream& operator<<(std::ostream& s, const Region& region);

}}
