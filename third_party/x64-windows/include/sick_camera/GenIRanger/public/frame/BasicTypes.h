// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include <string>

namespace geniranger { namespace frame {

/**
 * Identifier for different regions. Values are the same as for the GenICam
 * RegionsSelector to avoid confusion but you should not rely on this in your
 * code.
 */
enum class RegionId
{
  REGION_0 = 0,
  REGION_1 = 1,
  REGION_2 = 2,
  REGION_3 = 3,
  REGION_4 = 4,
  REGION_5 = 5,
  SCAN_3D_EXTRACTION_1 = 11,
  SCAN_3D_EXTRACTION_2 = 12,
  SCAN_3D_EXTRACTION_3 = 13,
  SCAN_3D_EXTRACTION_4 = 14,
  SCAN_3D_EXTRACTION_5 = 15,
};

/**
 * Identifier for different component types. Values are kept the same as for the
 * GenICam ComponentSelector to avoid confusion but you should not rely on this
 * in your code.
 */
enum class ComponentId
{
  INTENSITY = 0,
  RANGE = 1,
  REFLECTANCE = 2,
  SCATTER = 3,
  MAC0 = 4,
  MAC1 = 5,
  MAC2 = 6,
  GAIN = 7,
  PEAK2 = 8,
};

/** Whether the camera is configured to grab 2D gray images or 3D profiles. */
enum class DeviceScanType
{
  AREASCAN,
  LINESCAN_3D,
};

/**
 * Creates a \ref RegionId by parsing a string. The expected format is the one
 * used in GenApi, i.e., the one specified in the GenICam device description.
 */
RegionId regionIdFromString(const std::string& genapiName);

/**
 * Creates a \ref ComponentId by parsing a string. The expected format is the
 * one used in GenApi, i.e., the one specified in the GenICam device
 * description.
 */
ComponentId componentIdFromString(const std::string& genapiName);

/**
 * Creates a string from a \ref RegionId. Reverse operation of \ref
 * regionIdFromString.
 */
std::string toString(RegionId region);

/**
 * Creates a string from a \ref ComponentId. Reverse operation of \ref
 * componentIdFromString.
 */
std::string toString(ComponentId component);

}}
