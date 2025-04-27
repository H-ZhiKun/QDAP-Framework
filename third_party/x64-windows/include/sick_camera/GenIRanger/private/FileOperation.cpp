// Copyright 2017-2020 SICK AG. All rights reserved.

#include "../public/FileOperation.h"

#include "../public/Exceptions.h"
#include "../public/NodeHelpers.h"

#include <GenApi/Filestream.h>

#include <fstream>
#include <mutex>

// Flag to abort an ongoing file transfer
bool gRequestAbort = false;
// Mutex for making abort request thread-safe,
// protecting access to gRequestAbort
std::mutex abortMutex;

using namespace GenApi;

namespace geniranger {

// Local helper functions
namespace {

const char* readFile(const std::string& path, size_t& fileSize)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate | std::ios::in);
  if (!file.good())
  {
    std::stringstream ss;
    ss << "Unable to open file for reading: '" << path << "'";
    throw GenIRangerException(ss.str());
  }
  std::ifstream::pos_type pos = file.tellg();
  if (pos == std::ifstream::pos_type(-1))
  {
    std::stringstream ss;
    ss << "Unable to tell file size: '" << path << "'";
    throw GenIRangerException(ss.str());
  }
  fileSize = static_cast<size_t>(pos);
  file.seekg(0, std::ios::beg);

  char* buffer = new char[fileSize];
  if (file.read(buffer, fileSize))
  {
    return buffer;
  }
  else
  {
    delete[] buffer;
    std::stringstream ss;
    ss << "Unable to read file '" << path << "' with size " << fileSize << ".";
    throw GenIRangerException(ss.str());
  }
}

void writeFile(char* buffer,
               size_t bufferSize,
               const std::string& localFilePath)
{
  std::ofstream file(localFilePath,
                     std::ios::binary | std::ios::ate | std::ios::out);
  if (!file.good())
  {
    std::stringstream ss;
    ss << "Unable to open file for writing: '" << localFilePath << "'";
    throw GenIRangerException(ss.str());
  }
  file.write(buffer, bufferSize);
}

void tryClosingFile(IFileProtocolAdapter& adapter, const std::string& file)
{
  // Ignore return value
  adapter.closeFile(file.c_str());
}

}

GENIRANGER_API void sendFile(const std::string& sourceFilePath,
                             const std::string& deviceFile,
                             GenApi::INodeMap* const nodeMap,
                             GenApi::IFileProtocolAdapter& fileProtocolAdapter)
{
  size_t fileSize;
  std::unique_ptr<const char> rawContents(readFile(sourceFilePath, fileSize));
  std::string contents(rawContents.get(), fileSize);
  sendFileContents(contents, deviceFile, nodeMap, fileProtocolAdapter);
}

GENIRANGER_API void
sendFileContents(const std::string& contents,
                 const std::string& deviceFile,
                 GenApi::INodeMap* const nodeMap,
                 GenApi::IFileProtocolAdapter& fileProtocolAdapter)
{
  bool success = fileProtocolAdapter.attach(nodeMap);
  if (!success)
  {
    throw GenIRangerException("Unable to attach node map");
  }

  size_t byteCount = contents.size();

  success =
    fileProtocolAdapter.openFile(deviceFile.c_str(), std::ios_base::trunc);
  if (!success)
  {
    std::stringstream ss;
    ss << "Unable to open remote file '" << deviceFile << "'.";
    throw GenIRangerException(ss.str());
  }

  size_t bytesTransfered = 0;
  bool aborted = false;
  bool failedOrAborted = false;

  do
  {
    size_t remaningBytes = byteCount - bytesTransfered;
    // Transfer chunks to make it possible to cleanly abort the file transfer
    size_t chunk = 4096;
    if (remaningBytes < chunk)
    {
      chunk = remaningBytes;
    }
    size_t writtenBytes = static_cast<size_t>(
      fileProtocolAdapter.write(contents.data() + bytesTransfered,
                                bytesTransfered,
                                chunk,
                                deviceFile.c_str()));

    if (writtenBytes <= 0)
    {
      std::cout << "Error during file upload. Transferred " << bytesTransfered
                << " bytes so far." << std::endl;
      tryClosingFile(fileProtocolAdapter, deviceFile);
      throw GenIRangerException("Error while writing file");
    }
    else
    {
      bytesTransfered += writtenBytes;
    }

    std::lock_guard<std::mutex> lock(abortMutex);
    aborted = gRequestAbort;
    failedOrAborted = gRequestAbort;
  } while (bytesTransfered < byteCount && !aborted);

  {
    std::lock_guard<std::mutex> lock(abortMutex);
    gRequestAbort = false;
  }

  std::stringstream ss;
  if (aborted)
  {
    ss << "File transfer of '" << deviceFile << "' was aborted.";
  }
  else if (bytesTransfered != byteCount)
  {
    failedOrAborted = true;
    ss << "Unable to write remote file '" << deviceFile << "'.";
  }

  tryClosingFile(fileProtocolAdapter, deviceFile);
  if (failedOrAborted)
  {
    throw GenIRangerException(ss.str());
  }
}

GENIRANGER_API void abortSendFile()
{
  std::lock_guard<std::mutex> lock(abortMutex);
  gRequestAbort = true;
}

GENIRANGER_API void retrieveFile(const std::string& deviceFile,
                                 const std::string& destinationFilePath,
                                 GenApi::INodeMap* const nodeMap)
{
  auto contents = retrieveFileContents(deviceFile, nodeMap);

  std::ofstream outputFile;
  outputFile.open(destinationFilePath.c_str(),
                  std::ios_base::trunc | std::ios_base::binary);
  if (!outputFile.good())
  {
    std::stringstream ss;
    ss << "Unable to open destination file '" << destinationFilePath << "'.";
    throw GenIRangerException(ss.str());
  }

  outputFile << contents;

  if (!outputFile.good())
  {
    std::stringstream ss;
    ss << "Error during retrieval of '" << deviceFile << "'.";
    throw GenIRangerException(ss.str());
  }
}

GENIRANGER_API std::string retrieveFileContents(const std::string& deviceFile,
                                                GenApi::INodeMap* const nodeMap)
{
  GenApi::IDevFileStreamBuf<char, std::char_traits<char>> remoteFile;
  auto res = remoteFile.open(
    nodeMap, deviceFile.c_str(), std::ios_base::in | std::ios_base::binary);
  if (res == nullptr)
  {
    std::stringstream ss;
    ss << "Unable to open device file '" << deviceFile << "'.";
    throw GenIRangerException(ss.str());
  }
  std::stringstream contents;
  contents << &remoteFile;
  remoteFile.close();

  return contents.str();
}

GENIRANGER_API void
deleteFile(const std::string& deviceFile,
           GenApi::INodeMap* const nodeMap,
           GenApi::IFileProtocolAdapter& fileProtocolAdapter)
{
  bool success = fileProtocolAdapter.attach(nodeMap);
  if (!success)
  {
    throw GenIRangerException("Unable to attach node map");
  }

  success = fileProtocolAdapter.deleteFile(deviceFile.c_str());
  if (!success)
  {
    std::stringstream ss;
    ss << "Unable to delete remote file '" << deviceFile << "'.";
    throw GenIRangerException(ss.str());
  }
}

GENIRANGER_API void 
updateFirmware(const std::string& sourceFilePath,
                                   uint32_t crc32,
                                   GenApi::CNodeMapRef nodeMap)
{
  GenApi::FileProtocolAdapter fileProtocolAdapter;
  const std::string updateFileGeniName("Update");

  sendFile(
    sourceFilePath, updateFileGeniName, nodeMap._Ptr, fileProtocolAdapter);

  // Set Checksum register
  if (isInt(nodeMap, "FirmwareChecksum"))
  {
    setInt(nodeMap, "FirmwareChecksum", crc32);
  }
  else
  {
    throw GenIRangerException("FirmwareChecksum node not found in node map.");
  }

  // Trigger update
  if (isCommand(nodeMap, "FirmwarePerformUpdate"))
  {
    // Note: Device will reboot at the end of the update, invalidating the
    // nodemap. Therefore, do not poll it for completion.
    executeCommand(nodeMap, "FirmwarePerformUpdate", false);
  }
  else
  {
    throw GenIRangerException(
      "FirmwarePerformUpdate node not found in node map.");
  }
}

}
