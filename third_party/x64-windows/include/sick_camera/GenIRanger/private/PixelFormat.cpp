// Copyright 2019-2020 SICK AG. All rights reserved.
#include "../public/PixelFormat.h"

#include <sstream>

std::string geniranger::toString(PixelFormat format)
{
  switch (format)
  {
  case PixelFormat::MONO_8: return "MONO_8";
  case PixelFormat::MONO_12P: return "MONO_12P";
  case PixelFormat::MONO_16: return "MONO_16";
  case PixelFormat::COORD_3D_C8: return "COORD_3D_C8";
  case PixelFormat::COORD_3D_C12P: return "COORD_3D_C12P";
  case PixelFormat::COORD_3D_C16: return "COORD_3D_C16";
  case PixelFormat::COORD_3D_A16: return "COORD_3D_A16";
  case PixelFormat::MONO_4P: return "MONO_4P";
  }
  std::ostringstream ss;
  ss << "Unknown PixelFormat: " << static_cast<uint64_t>(format);
  return ss.str();
}
