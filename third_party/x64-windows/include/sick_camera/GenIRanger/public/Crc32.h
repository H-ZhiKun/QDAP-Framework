// Copyright 2020 SICK AG. All rights reserved.

#pragma once

#include <cstdint>
#include <istream>
#include "GenIRangerDll.h"// VLB Modified

namespace geniranger {

/**
 * Helper class to calculate the CRC32 checksum using the polynomial used by
 * Ethernet/PKZip.
 */
class Crc32
{
public:
  GENIRANGER_API Crc32();

  /** Calculates the CRC32 checksum for the contents of the stream. */
  GENIRANGER_API uint32_t calculate(std::istream& stream) const;

  /** Calculates the CRC32 checksum for the contents of a file. */
  GENIRANGER_API uint32_t calculate(const std::string& path) const;

private:
  uint32_t mLookup[256];
};

}
