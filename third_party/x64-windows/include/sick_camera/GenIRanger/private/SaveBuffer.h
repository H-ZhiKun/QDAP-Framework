// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include "../public/StreamData.h"

#include <stdint.h>
#include <string>

namespace geniranger {

class SaveBuffer
{
private:
  static const float SIXTEENTH() { return 1.0f / 16; }

public:
  /** One metadata value. */
  typedef uint32_t MarkdataWord;

  /** Calculates ICon file format sensor trait OriginZ. */
  static double calculateOriginZ(const int64_t aoiOffsetY,
                                 const int64_t aoiHeight,
                                 RangeAxis rangeAxis)
  {
    // Regarding calculation, see comments in RNX-3940
    if (rangeAxis == RangeAxis::REVERSE)
    {
      return 1.0 * aoiOffsetY + aoiHeight;
    }
    else
    {
      return 1.0 * aoiOffsetY;
    }
  }

  /** Calculates ICon file format sensor trait ScaleZ. */
  static double calculateScaleZ(RangeAxis rangeAxis)
  {
    auto sign = rangeAxis == RangeAxis::REVERSE ? -1 : 1;
    return sign * SIXTEENTH();
  }
};

}
