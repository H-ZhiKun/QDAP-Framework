// Copyright 2017-2020 SICK AG. All rights reserved.

#pragma once

#include "GenICam.h"
#include "GenIRangerDll.h"

namespace GENAPI_NAMESPACE {
struct IFileProtocolAdapter;
}

namespace geniranger {

/**
 * Sends a file to the device.
 *
 * \param sourceFilePath path to the local file that should be sent to the
 *        device
 * \param deviceFile the name of the GenICam file on the device, i.e., the
 *        FileSelector
 * \param nodeMap the node map
 * \param fileProtocolAdapter adapter between the std::iostreambuf and the SFNC
 *        Features representing the device file system
 */
GENIRANGER_API void sendFile(const std::string& sourceFilePath,
                             const std::string& deviceFile,
                             GenApi::INodeMap* const nodeMap,
                             GenApi::IFileProtocolAdapter& fileProtocolAdapter);

/**
 * Sends contents to a device file.
 *
 * \param contents the contents to write to the device file
 * \param deviceFile the name of the GenICam file on the device, i.e., the
 *        FileSelector
 * \param nodeMap the node map
 * \param fileProtocolAdapter adapter between the std::iostreambuf and the SFNC
 *        Features representing the device file system
 */
GENIRANGER_API void
sendFileContents(const std::string& contents,
                 const std::string& deviceFile,
                 GenApi::INodeMap* const nodeMap,
                 GenApi::IFileProtocolAdapter& fileProtocolAdapter);

/** Aborts ongoing \ref sendFile() operation */
GENIRANGER_API void abortSendFile();

/**
 * Retrieves a file from the device.
 *
 * \param deviceFile the name of the GenICam file on the device, i.e., the
 *        FileSelector
 * \param destinationFilePath path to the local destination file
 * \param nodeMap the node map
 */
GENIRANGER_API void retrieveFile(const std::string& deviceFile,
                                 const std::string& destinationFilePath,
                                 GenApi::INodeMap* const nodeMap);

/**
 * Retrieves the contents of a camera (device) file.
 *
 * \param deviceFile the name of the GenICam file on the device, i.e., the
 *        FileSelector
 * \param nodeMap the node map
 * \return the contents to read from the device file
 */
GENIRANGER_API std::string
retrieveFileContents(const std::string& deviceFile,
                     GenApi::INodeMap* const nodeMap);

/**
 * Removes a file on the device.
 *
 * \param deviceFile the name of the target file on the device.
 * \param nodeMap the node map
 * \param fileProtocolAdapter adapter between the std::iostreambuf and the SFNC
 *        Features representing the device file system
 */
GENIRANGER_API void
deleteFile(const std::string& deviceFile,
           GenApi::INodeMap* const nodeMap,
           GenApi::IFileProtocolAdapter& fileProtocolAdapter);

/**
 * Updates firmware with the specified package.
 *
 * \param sourceFilePath path to the firmware package
 * \param crc32 the CRC checksum of the firmware package
 * \param nodeMap the node map
 */
GENIRANGER_API void updateFirmware(const std::string& sourceFilePath,
                                   uint32_t crc32,
                                   GenApi::CNodeMapRef nodeMap);
}
