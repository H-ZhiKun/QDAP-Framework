// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

#include "GenIRangerDll.h"
#include <GenICam.h>
#include <fstream>
#include <string>

namespace geniranger {

/**
 * Upon creation this class registers a callback to LogMessageEventID in order
 * to know when a new log message has been pushed into the device node map. It
 * will read LogMessageText, LogMessageLevel and LogMessageTimestamp and output
 * their values to a log file specified in constructor.
 *
 * A prerequisite for this class to work is to attach the node map to a
 * GenApi::EventAdapterGEV. It will take a GenTL event buffer and extract its
 * contents into the node map.
 *
 * The log has a limit to it, when this is reached it will be 'copied' by being
 * renamed with the added suffix '-backup'. It will then continue logging to a
 * new file with the original path.
 */
class DeviceLogWriter
{
public:
  /**
   * Constructs logger and starts logging device events immediately. Logging
   * stops when destructor is called.
   *
   * \param device Node map of the device.
   * \param path Path where to store the log file, including name of the file.
   * \param limit Size limit of log file in bytes. Only when this has been
   *        exceeded will its contents be copied to a backup before being
   *        cleared.
   */
  GENIRANGER_API
  DeviceLogWriter(GenApi::INodeMap* device, std::string path, int64_t limit);
  GENIRANGER_API ~DeviceLogWriter();

  /** Turn on/off logging to standard error (console). */
  void GENIRANGER_API enableConsoleLog(bool enable);

private:
  /**
   * In the case of exceptions when writing to disk this method will print to
   * std::cerr. This is done to avoid throwing exceptions when the callback
   * functions for GenApi are triggered, which could lead to undesired effects.
   */
  void logToFile(GenApi::INode* /*node*/);

  /**
   * True if the file currently being written has reached or exceeded its
   * maximum size.
   */
  bool isMaxFileSizeReached();

  /**
   * Creates a backup of the log by renaming it with the suffix '-backup', and
   * then creating a new file with the original path.
   */
  void copyLogToBackup();

private:
  std::string mFilePath;
  std::string mBackupPath;
  int64_t mLimit;
  std::ofstream mLogStream;
  intptr_t mCallbackHandle;
  GenApi::INodeMap* mDevice;
  GenApi::CStringPtr mLogText;
  GenApi::CIntegerPtr mLogLevel;
  GenApi::CIntegerPtr mTime;
  GenApi::CIntegerPtr mLegacyTimeHigh;
  GenApi::CIntegerPtr mLegacyTimeLow;
  bool mConsoleLogging;
};

}
