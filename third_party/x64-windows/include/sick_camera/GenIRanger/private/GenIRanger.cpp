// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../public/GenIRanger.h"
#include "../public/Exceptions.h"
#include "NodeExporter.h"
#include "../public/NodeHelpers.h"
#include "NodeImporter.h"
#include "../public/NodeMap.h"
#include "SelectorSnapshot.h"
#include "../public/UserSet.h"

#include <GenApi/Filestream.h>

#include <iostream>
#include <sstream>
#include <string>

using namespace GenApi;

namespace {

void appendLines(std::ostringstream& out,
                 const std::vector<std::string>& strings)
{
  for (auto& string : strings)
  {
    out << string << "\n";
  }
}

}

namespace geniranger {

/**
 * Unlike the NodeMap class, this sub-class doesn't ensure the combined
 * ownership of a GenApi node map and the its port. This is useful when using
 * GenIRanger from Java.
 */
class UnmanagedNodeMap : public NodeMap
{
public:
  UnmanagedNodeMap(GenApi::CNodeMapRef nodeMap)
    : NodeMap(nodeMap)
  {
  }
};


GENIRANGER_API void exportDeviceParameters(INodeMap* const nodeMap,
                                           std::ostream& outputCsv)
{
  CCommandPtr start = nodeMap->GetNode("DeviceFeaturePersistenceStart");
  CCommandPtr stop = nodeMap->GetNode("DeviceFeaturePersistenceStop");
  CCategoryPtr root = nodeMap->GetNode("Root");
  CIntegerPtr locked = nodeMap->GetNode("TLParamsLocked");

  if (locked && locked->GetValue() == 1)
  {
    throw ExportException("Cannot export while parameters are locked.");
  }

  // Prepare device for persisting of all streamable features
  if (start)
  {
    start->Execute();
  }

  ConfigWriter formatter(outputCsv);
  NodeExporter exporter(formatter);

  try
  {
    SelectorSnapshot snapshot(nodeMap);
    exporter.traverse(root->GetNode());
  }
  catch (GenICam::GenericException& e)
  {
    throw ExportException(e.what());
  }
  catch (std::exception& e)
  {
    throw ExportException(e.what());
  }

  if (stop)
  {
    stop->Execute();
  }

  const auto& errors = exporter.getErrors();
  if (!errors.empty())
  {
    std::stringstream errorDetails;
    errorDetails << "Could not export configuration from device:\n";
    for (size_t i = 0; i < errors.size(); ++i)
    {
      errorDetails << errors[i] << "\n";
    }
    throw ExportException(errorDetails.str());
  }
}


GENIRANGER_API void exportDeviceParametersVLB(INodeMap * const nodeMap, std::ostream & outputCsv)
{
	CCommandPtr start = nodeMap->GetNode("DeviceFeaturePersistenceStart");
	CCommandPtr stop = nodeMap->GetNode("DeviceFeaturePersistenceStop");
	CCategoryPtr root = nodeMap->GetNode("Root");
	CIntegerPtr locked = nodeMap->GetNode("TLParamsLocked");

	if (locked && locked->GetValue() == 1)
	{
		throw ExportException("Cannot export while parameters are locked.");
	}

	// Prepare device for persisting of all streamable features
	if (start)
	{
		start->Execute();
	}

	ConfigWriterVLB formatter(outputCsv); //// modified by VLB
	NodeExporterVLB exporter(formatter);  //// modified by VLB

	try
	{
		SelectorSnapshot snapshot(nodeMap);
		exporter.traverse(root->GetNode());
	}
	catch (GenICam::GenericException& e)
	{
		throw ExportException(e.what());
	}
	catch (std::exception& e)
	{
		throw ExportException(e.what());
	}

	if (stop)
	{
		stop->Execute();
	}

	const auto& errors = exporter.getErrors();
	if (!errors.empty())
	{
		std::stringstream errorDetails;
		errorDetails << "Could not export configuration from device:\n";
		for (size_t i = 0; i < errors.size(); ++i)
		{
			errorDetails << errors[i] << "\n";
		}
		throw ExportException(errorDetails.str());
	}
}



GENIRANGER_API ConfigurationResult
importDeviceParameters(INodeMap* const nodeMap, std::istream& inputCsv)
{
  CCommandPtr start = nodeMap->GetNode("DeviceRegistersStreamingStart");
  CCommandPtr stop = nodeMap->GetNode("DeviceRegistersStreamingEnd");
  CBooleanPtr registersValidNode = nodeMap->GetNode("DeviceRegistersValid");
  CCategoryPtr root = nodeMap->GetNode("Root");

  ConfigReader reader;
  ConfigurationResult result = reader.parse(inputCsv);
  if (result.status != ConfigurationStatus::OK)
  {
    return result;
  }

  // Prepare device for registers streaming without checking for consistency
  if (start)
  {
    start->Execute();
  }

  NodeImporter importer(reader);

  try
  {
    SelectorSnapshot snapshot(nodeMap);
    importer.traverse(root->GetNode());
  }
  catch (GenICam::GenericException& e)
  {
    return ConfigurationResult(ConfigurationStatus::ERROR_GENERIC, e.what());
  }
  catch (std::exception& e)
  {
    return ConfigurationResult(ConfigurationStatus::ERROR_GENERIC, e.what());
  }

  if (stop)
  {
    stop->Execute();
  }

  // Check the DeviceRegistersValid if the feature exists
  bool invalidStateOnDevice =
    (registersValidNode && registersValidNode->GetValue() == false);

  result.missingParameters = importer.getMissing();
  result.unknownParameters = importer.getUnknown();
  if (result.hasMissing() || result.hasUnknown())
  {
    result.status = ConfigurationStatus::WARNING;
  }
  if (result.hasMissing() && result.hasUnknown())
  {
    result.message = "The configuration is missing some parameters and "
                     "contains some unknown parameters";
  }
  else if (result.hasMissing())
  {
    result.message = "The configuration is missing some parameters";
  }
  else if (result.hasUnknown())
  {
    result.message = "The configuration contains some unknown parameters";
  }

  const auto& errors = importer.getErrors();
  if (!errors.empty() || invalidStateOnDevice)
  {
    std::ostringstream errorDetails;
    errorDetails << "Could not import configuration to device:\n";

    if (invalidStateOnDevice)
    {
      errorDetails << "The device says the configuration is invalid "
                   << "(DeviceRegistersValid=false)\n";
    }

    appendLines(errorDetails, errors);
    result = ConfigurationResult(
      ConfigurationStatus::ERROR_INVALID_CONFIGURATION, errorDetails.str());
  }

  return result;
}

GENIRANGER_API bool userSetNeedsMigration(GenApi::CNodeMapRef nodeMap,
                                          const std::string& userSetName)
{
  geniranger::UserSet userSet(UnmanagedNodeMap(nodeMap),
                              geniranger::userSetIdFromString(userSetName));
  return userSet.needsMigration();
}

GENIRANGER_API ConfigurationResult
migrateUserSet(GenApi::CNodeMapRef nodeMap, const std::string& userSetName)
{
  geniranger::UserSet userSet(UnmanagedNodeMap(nodeMap),
                              geniranger::userSetIdFromString(userSetName));
  return userSet.migrate();
}

GENIRANGER_API void convert12pTo16(const uint8_t* inBuffer,
                                   const int64_t inSize,
                                   uint8_t* outBuffer,
                                   int64_t* inOutSize)
{
  int64_t i = 0;
  int64_t bytesWritten = 0;
  do
  {
    uint8_t b1 = *inBuffer++;
    uint8_t b2 = *inBuffer++;
    uint8_t b3 = *inBuffer++;

    // Pixel 1
    uint8_t bo1 = b1;
    uint8_t bo2 = b2 & 0x0F;

    // Pixel 2
    uint8_t bo3 = ((b2 & 0xF0) >> 4) | (b3 & 0x0F) << 4;
    uint8_t bo4 = (b3 & 0xF0) >> 4;

    bytesWritten = bytesWritten + 4;
    if (bytesWritten > *inOutSize)
    {
      std::stringstream ss;
      ss << "Buffer overrun, provided buffer with size " << *inOutSize
         << " cannot hold the unpacked data";
      throw GenIRangerException(ss.str());
    }
    *outBuffer++ = bo1;
    *outBuffer++ = bo2;
    *outBuffer++ = bo3;
    *outBuffer++ = bo4;

    i = i + 3;
  } while (i < inSize);
  *inOutSize = bytesWritten;
}

GENIRANGER_API void convert16To12p(const uint16_t* inBuffer,
                                   const int64_t inSize,
                                   uint8_t* outBuffer,
                                   int64_t* inOutSize)
{
  if (inSize % 2 != 0)
  {
    throw GenIRangerException("Size of a 16 bit input buffer must be even");
  }

  uint8_t packedBytes[3];
  int64_t outsize = 0;
  int64_t numBytes = 0;
  uint8_t packed = 0;

  do
  {
    // LSB oriented version
    if (packed == 0)
    {
      // 8 lsb bits to byte 0
      packedBytes[0] = *inBuffer & 0x00FF;

      // 4 msb bits to byte 1, low nibble
      packedBytes[1] = (*inBuffer & 0x0F00) >> 8;
      packed = 2;
    }
    else
    {
      // 4 lsb bits to byte 1, high nibble
      packedBytes[1] = packedBytes[1] | (*inBuffer & 0x000F) << 4;

      // 8 msb bits to byte 2
      packedBytes[2] = static_cast<uint8_t>((*inBuffer & 0xFFF0) >> 4);

      outsize = outsize + 3;
      if (outsize > *inOutSize)
      {
        throw GenIRangerException("Output buffer size insufficient");
      }
      // 3 bytes ready, write out
      for (uint8_t k = 0; k < 3; ++k)
      {
        *outBuffer++ = packedBytes[k];
      }
      packed = 0;
    }
    inBuffer++;
    numBytes = numBytes + 2;
  } while (numBytes < inSize);
  *inOutSize = outsize;
}

}
