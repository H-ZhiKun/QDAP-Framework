// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../public/DeviceLogWriter.h"
#include <cstdio>
#include <iostream>
#include <sys/stat.h>

namespace geniranger {

DeviceLogWriter::DeviceLogWriter(GenApi::INodeMap* device,
                                 std::string path,
                                 int64_t limit)
  : mDevice(device)
  , mFilePath(path)
  , mBackupPath(path + "-backup")
  , mLimit(limit)
  , mConsoleLogging(false)
{
  mLogText = mDevice->GetNode("LogMessageText");
  mLogLevel = mDevice->GetNode("LogMessageLevel");
  mTime = mDevice->GetNode("LogMessageTimestamp");
  mLegacyTimeLow = mDevice->GetNode("LogMessageTimestampLow");
  mLegacyTimeHigh = mDevice->GetNode("LogMessageTimestampHigh");

  // Register a callback on the event node which triggers a callback when
  // EventAdapterGEV#DeliverMessage is called with the event buffer
  GenApi::CNodePtr logMessage = mDevice->GetNode("LogMessageEventID");
  mCallbackHandle =
    GenApi::Register(logMessage, *this, &DeviceLogWriter::logToFile);

  mLogStream = std::ofstream(mFilePath.c_str(), std::ios::app | std::ios::ate);
  mLogStream.exceptions(std::ios::failbit | std::ios::badbit);
}

DeviceLogWriter::~DeviceLogWriter()
{
  GenApi::Deregister(mCallbackHandle);
  mLogStream.exceptions(std::ios::goodbit);
  mLogStream.close();
};

bool DeviceLogWriter::isMaxFileSizeReached()
{
  return mLogStream.tellp() > mLimit;
}

inline boolean fileExists(std::string& path)
{
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

void DeviceLogWriter::copyLogToBackup()
{
  try
  {
    mLogStream.close();
    if (fileExists(mBackupPath))
    {
      int error = std::remove(mBackupPath.c_str());
      if (error)
      {
        std::perror("Could not remove backup log file");
      }
    }
    int error = std::rename(mFilePath.c_str(), mBackupPath.c_str());
    if (error)
    {
      std::perror("Could not backup log file");
    }
    mLogStream.open(mFilePath.c_str());
  }
  catch (std::exception& e)
  {
    std::cerr << "Cannot copy log to backup file: " << e.what() << std::endl;
  }
}

void DeviceLogWriter::enableConsoleLog(bool enable)
{
  mConsoleLogging = enable;
}


void DeviceLogWriter::logToFile(GenApi::INode* /*node*/)
{
  std::string text(mLogText->GetValue());
  int64_t level = mLogLevel->GetValue();
  uint64_t timestamp;

  if (mTime)
  {
    timestamp = mTime->GetValue();
  }
  else
  {
    // For devices with firmware version 1.1 SR2 and below.
    uint32_t high = static_cast<uint32_t>(mLegacyTimeHigh->GetValue());
    uint32_t low = static_cast<uint32_t>(mLegacyTimeLow->GetValue());
    timestamp = (static_cast<uint64_t>(high) << 32) | low;
  }

  std::string levelString;
  switch (level)
  {
  case 0: levelString = "Fine"; break;
  case 1: levelString = "Info"; break;
  case 2: levelString = "Warning"; break;
  case 3: levelString = "Severe"; break;
  default:
    std::cerr << "Cannot map event level to corresponding string" << std::endl;
    break;
  }
  try
  {
    if (isMaxFileSizeReached())
    {
      copyLogToBackup();
    }
    mLogStream << levelString << " " << timestamp << " " << text << std::endl;
    if (mConsoleLogging)
    {
      std::cerr << levelString << " " << timestamp << " " << text << std::endl;
    }
  }
  catch (std::exception& e)
  {
    mLogStream.clear();
    std::cerr << e.what() << std::endl;
  }
}

}
