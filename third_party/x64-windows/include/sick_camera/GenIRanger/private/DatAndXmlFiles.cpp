// Copyright 2018-2020 SICK AG. All rights reserved.

#include "DatAndXmlFiles.h"

namespace geniranger {

DatAndXmlFiles::DatAndXmlFiles(std::string filePathWithNoEnding)
  : mDataStream(filePathWithNoEnding + ".dat", openMode)
  , mXmlStream(filePathWithNoEnding + ".xml", openMode)
{
  mDataStream.exceptions(std::ios::failbit | std::ios::badbit);
  mXmlStream.exceptions(std::ios::failbit | std::ios::badbit);
}

DatAndXmlFiles::~DatAndXmlFiles()
{
  mDataStream.close();
  mXmlStream.close();
}


std::ostream& DatAndXmlFiles::xmlStream()
{
  return mXmlStream;
}

std::ostream& DatAndXmlFiles::dataStream()
{
  return mDataStream;
}

}
