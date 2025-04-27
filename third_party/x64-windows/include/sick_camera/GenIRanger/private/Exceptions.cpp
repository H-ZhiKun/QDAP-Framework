// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../public/Exceptions.h"

namespace geniranger {

GenIRangerException::GenIRangerException(const std::string& message)
  : std::exception(message.c_str())
{
  // Empty
}

ExportException::ExportException(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

DeviceLogException::DeviceLogException(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

SaveException::SaveException(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

XmlAttributeException::XmlAttributeException(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

NotAvailableException::NotAvailableException(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

InvalidIpAddress::InvalidIpAddress(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

InvalidMacAddress::InvalidMacAddress(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

ChunkFormatException::ChunkFormatException(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

InvalidConfiguration::InvalidConfiguration(const std::string& message)
  : GenIRangerException(message)
{
  // Empty
}

}
