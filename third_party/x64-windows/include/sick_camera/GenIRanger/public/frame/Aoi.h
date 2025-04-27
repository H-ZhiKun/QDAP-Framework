// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once


namespace geniranger { namespace frame {

/**
 * Represents an area of interest on the sensor, in pixel coordinates relative
 * to the top left corner.
 */
struct Aoi
{
  Aoi()
    : Aoi(0, 0, 0, 0)
  {
  }

  Aoi(uint16_t offsetX, uint16_t offsetY, uint16_t width, uint16_t height)
    : offsetX(offsetX)
    , offsetY(offsetY)
    , width(width)
    , height(height)
  {
  }

  uint16_t offsetX;
  uint16_t offsetY;
  uint16_t width;
  uint16_t height;
};

}}
