// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include <cstdint>
#include <string>

namespace geniranger {

// From GenICam Pixel Format Naming Convention we get the
// pixel format values, imported from "PFNC.h" available at emva.org
enum class PixelFormat : uint64_t
{
  // Monochrome 8-bit
  MONO_8 = 0x01080001,
  // Monochrome 12-bit packed
  MONO_12P = 0x010C0047,
  // Monochrome 16-bit
  MONO_16 = 0x01100007,
  // 3D coordinate C 8-bit
  COORD_3D_C8 = 0x010800B1,
  // 3D coordinate C 12-bit packed
  COORD_3D_C12P = 0x010C00DA,
  // 3D coordinate C 16-bit
  COORD_3D_C16 = 0x011000B8,
  // 3D coordinate A 16-bit (calibrated)
  COORD_3D_A16 = 0x011000B6,
  // Monochrome 4-bit packed
  MONO_4P = 0x01040039,
};

std::string toString(PixelFormat format);

}
