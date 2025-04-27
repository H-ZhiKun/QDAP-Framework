// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include "../public/StreamData.h"

#include <fstream>
#include <functional>
#include <string>

namespace geniranger {

/**
 * The file format written to requires two files. A DAT-file for binary data and
 * a XML-file for descriptive information.
 */
class DatAndXmlFiles
{
private:
  std::ofstream mDataStream;
  std::ofstream mXmlStream;
  static const auto openMode =
    std::ios::binary | std::ios::trunc | std::ios::out;

public:
  DatAndXmlFiles(std::string filePathWithNoEnding);

  virtual ~DatAndXmlFiles();

  std::ostream& xmlStream();

  std::ostream& dataStream();
};

}
