// Copyright 2019-2020 SICK AG. All rights reserved.

#include "../../public/frame/BasicTypes.h"
#include <stdexcept>

namespace geniranger { namespace frame {

RegionId regionIdFromString(const std::string& genapiName)
{
  if (genapiName == "Region0")
  {
    return RegionId::REGION_0;
  }
  else if (genapiName == "Region1")
  {
    return RegionId::REGION_1;
  }
  else if (genapiName == "Region2")
  {
    return RegionId::REGION_2;
  }
  else if (genapiName == "Region3")
  {
    return RegionId::REGION_3;
  }
  else if (genapiName == "Region4")
  {
    return RegionId::REGION_4;
  }
  else if (genapiName == "Region5")
  {
    return RegionId::REGION_5;
  }
  else if (genapiName == "Scan3dExtraction1")
  {
    return RegionId::SCAN_3D_EXTRACTION_1;
  }
  else if (genapiName == "Scan3dExtraction2")
  {
    return RegionId::SCAN_3D_EXTRACTION_2;
  }
  else if (genapiName == "Scan3dExtraction3")
  {
    return RegionId::SCAN_3D_EXTRACTION_3;
  }
  else if (genapiName == "Scan3dExtraction4")
  {
    return RegionId::SCAN_3D_EXTRACTION_4;
  }
  else if (genapiName == "Scan3dExtraction5")
  {
    return RegionId::SCAN_3D_EXTRACTION_5;
  }
  throw std::invalid_argument(std::string("Unknown region: ") + genapiName);
}

ComponentId componentIdFromString(const std::string& genapiName)
{
  if (genapiName == "Range")
  {
    return ComponentId::RANGE;
  }
  else if (genapiName == "Reflectance")
  {
    return ComponentId::REFLECTANCE;
  }
  else if (genapiName == "Intensity")
  {
    return ComponentId::INTENSITY;
  }
  else if (genapiName == "Scatter")
  {
    return ComponentId::SCATTER;
  }
  else if (genapiName == "Mac0")
  {
    return ComponentId::MAC0;
  }
  else if (genapiName == "Mac1")
  {
    return ComponentId::MAC1;
  }
  else if (genapiName == "Mac2")
  {
    return ComponentId::MAC2;
  }
  else if (genapiName == "Gain")
  {
    return ComponentId::GAIN;
  }
  else if (genapiName == "Peak2")
  {
    return ComponentId::PEAK2;
  }
  throw std::invalid_argument(std::string("Unknown component: ") + genapiName);
}

std::string toString(RegionId region)
{
  switch (region)
  {
  case RegionId::REGION_0: return "Region0";
  case RegionId::REGION_1: return "Region1";
  case RegionId::REGION_2: return "Region2";
  case RegionId::REGION_3: return "Region3";
  case RegionId::REGION_4: return "Region4";
  case RegionId::REGION_5: return "Region5";
  case RegionId::SCAN_3D_EXTRACTION_1: return "Scan3dExtraction1";
  case RegionId::SCAN_3D_EXTRACTION_2: return "Scan3dExtraction2";
  case RegionId::SCAN_3D_EXTRACTION_3: return "Scan3dExtraction3";
  case RegionId::SCAN_3D_EXTRACTION_4: return "Scan3dExtraction4";
  case RegionId::SCAN_3D_EXTRACTION_5: return "Scan3dExtraction5";
  }
  throw std::invalid_argument("Unknown region id");
}

std::string toString(ComponentId component)
{
  switch (component)
  {
  case ComponentId::INTENSITY: return "Intensity";
  case ComponentId::RANGE: return "Range";
  case ComponentId::REFLECTANCE: return "Reflectance";
  case ComponentId::SCATTER: return "Scatter";
  case ComponentId::MAC0: return "Mac0";
  case ComponentId::MAC1: return "Mac2";
  case ComponentId::MAC2: return "Mac2";
  case ComponentId::GAIN: return "Gain";
  case ComponentId::PEAK2: return "Peak2";
  }
  throw std::invalid_argument("Unknown component id");
}

}}
