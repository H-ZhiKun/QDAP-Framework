// Copyright 2020 SICK AG. All rights reserved.

#include "../public/Crc32.h"
#include "../public/Exceptions.h"

#include <fstream>
#include <sstream>

namespace geniranger {

    GENIRANGER_API Crc32::Crc32()
{
  const uint32_t reversedPolynomial = 0xedb88320L;

  for (int i = 0; i < 256; ++i)
  {
    uint32_t checksum = i;
    for (int n = 0; n < 8; ++n)
    {
      checksum = (checksum & 1 ? 0 : reversedPolynomial) ^ checksum >> 1;
    }
    checksum = checksum ^ static_cast<uint32_t>(0xff000000);
    mLookup[i] = checksum;
  }
}

    GENIRANGER_API uint32_t Crc32::calculate(std::istream& stream) const
{
  uint32_t checksum = 0;
  int c = 0;
  while ((c = stream.get()) != EOF)
  {
    checksum = mLookup[static_cast<uint8_t>(checksum) ^ static_cast<uint8_t>(c)]
               ^ (checksum >> 8);
  }
  return checksum;
}

    GENIRANGER_API uint32_t Crc32::calculate(const std::string& path) const
{
  std::ifstream fin(path, std::ifstream::binary);
  if (!fin.is_open())
  {
    std::ostringstream oss;
    oss << "Could not open " << path << " for CRC-32 calculation.";
    throw GenIRangerException(oss.str());
  }
  return calculate(fin);
}

}
