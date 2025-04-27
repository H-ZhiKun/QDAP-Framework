// Copyright 2019-2020 SICK AG. All rights reserved.

#include "../public/ConnectedDevice.h"
#include "../public/Crc32.h"
#include "../public/Exceptions.h"
#include "../public/FrameGrabber.h"
#include "../public/GenIRanger.h"
#include "../public/NodeHelpers.h"
#include "../public/gentlcpp/Device.h"

#include <GenApi/Filestream.h>
#include <thread>

namespace {

/** \return true if the string ends with the expected suffix */
bool endsWith(const std::string& s, const std::string& suffix)
{
  if (s.length() < suffix.length())
  {
    return false;
  }
  return s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
}

/** Wait until connection to device has been lost. */
void waitForDisconnect(GenApi::CNodeMapRef nodeMap)
{
  // Wait until we have lost connection for sure
  for (;;)
  {
    try
    {
      // Poll a parameter that is not cached, i.e., we know it will try to get
      // the value from the device.
      geniranger::getFloat(nodeMap, "DeviceTemperature");
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (gentlcpp::IoException&)
    {
      // This means connection is lost
      break;
    }
  }
}

void waitForFirmwareUpdateCompletion(GenApi::CNodeMapRef nodeMap)
{
  std::string status;
  try
  {
    while ((status = geniranger::getEnum(nodeMap, "FirmwareUpdateProgress"))
           != "Completed")
    {
      if (endsWith(status, "Error"))
      {
        throw geniranger::GenIRangerException("Firmware update failed with "
                                              + status);
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
  catch (gentlcpp::IoException& e)
  {
    // Note: Completed sometimes isn't reached since the device is rebooted by
    // the firmware update script. In that case the connection will be lost
    // but we should at least have reached the script execution phase.
    if (status != "ExecutingScript")
    {
      throw e;
    }
  }

  // If we reached the Completed status we still wait until the connection is
  // lost to the device to ensure we always are in the same state, regardless of
  // timing.
  waitForDisconnect(nodeMap);
}

}

namespace geniranger {

ConnectedDevice::ConnectedDevice(std::shared_ptr<gentlcpp::Device> device)
  : mDevice(device)
  , mDataStream(mDevice->openDataStream(mDevice->getDataStreamIds().at(0)))
  , mNodeMap(mDevice->getRemotePort())
{
  mRegisterStreamingActive = getBool("DeviceRegistersStreamingActive");
}

std::shared_ptr<FrameGrabber>
ConnectedDevice::createFrameGrabber(size_t buffersCount)
{
  return FrameGrabber::create(mNodeMap, mDataStream, buffersCount);
}

std::shared_ptr<gentlcpp::Device> ConnectedDevice::getDevice()
{
  return mDevice;
}

NodeMap ConnectedDevice::getNodeMap()
{
  return mNodeMap;
}

std::shared_ptr<gentlcpp::DataStream> ConnectedDevice::getDataStream()
{
  return mDataStream;
}

void ConnectedDevice::registerStreamingStart()
{
  if (mRegisterStreamingActive)
  {
    throw GenIRangerException("Cannot start register streaming when a "
                              "streaming session is already active");
  }
  executeCommand("DeviceRegistersStreamingStart");
  mRegisterStreamingActive = true;
}

void ConnectedDevice::registerStreamingEnd()
{
  if (!mRegisterStreamingActive)
  {
    throw GenIRangerException("Cannot end register streaming when no "
                              "streaming session is active");
  }
  executeCommand("DeviceRegistersStreamingEnd");
  mRegisterStreamingActive = false;

  if (!getBool("DeviceRegistersValid"))
  {
    throw InvalidConfiguration("The device has an invalid configuration after "
                               "ending register streaming");
  }
}

GenApi::EAccessMode
ConnectedDevice::getAccessMode(const std::string& parameterName) const
{
  return mNodeMap.getAccessMode(parameterName);
}

bool ConnectedDevice::isCommand(const std::string& parameterName) const
{
  return mNodeMap.isCommand(parameterName);
}

bool ConnectedDevice::isBool(const std::string& parameterName) const
{
  return mNodeMap.isBool(parameterName);
}

bool ConnectedDevice::isFloat(const std::string& parameterName) const
{
  return mNodeMap.isFloat(parameterName);
}

bool ConnectedDevice::isInt(const std::string& parameterName) const
{
  return mNodeMap.isInt(parameterName);
}

bool ConnectedDevice::isEnum(const std::string& parameterName) const
{
  return mNodeMap.isEnum(parameterName);
}

bool ConnectedDevice::isString(const std::string& parameterName) const
{
  return mNodeMap.isString(parameterName);
}

void ConnectedDevice::executeCommand(const std::string& parameterName,
                                     bool waitForCompletion)
{
  mNodeMap.executeCommand(parameterName, waitForCompletion);
}

bool ConnectedDevice::getBool(const std::string& parameterName) const
{
  return mNodeMap.getBool(parameterName);
}

float ConnectedDevice::getFloat(const std::string& parameterName) const
{
  return mNodeMap.getFloat(parameterName);
}

int64_t ConnectedDevice::getInt(const std::string& parameterName) const
{
  return mNodeMap.getInt(parameterName);
}

std::string ConnectedDevice::getEnum(const std::string& parameterName) const
{
  return mNodeMap.getEnum(parameterName);
}

int64_t ConnectedDevice::getEnumInt(const std::string& parameterName) const
{
  return mNodeMap.getEnumInt(parameterName);
}

std::string ConnectedDevice::getString(const std::string& parameterName) const
{
  return mNodeMap.getString(parameterName);
}

void ConnectedDevice::setBool(const std::string& parameterName, bool value)
{
  mNodeMap.setBool(parameterName, value, !mRegisterStreamingActive);
}

void ConnectedDevice::setFloat(const std::string& parameterName, float value)
{
  mNodeMap.setFloat(parameterName, value, !mRegisterStreamingActive);
}

void ConnectedDevice::setInt(const std::string& parameterName, int64_t value)
{
  mNodeMap.setInt(parameterName, value, !mRegisterStreamingActive);
}

void ConnectedDevice::setEnum(const std::string& parameterName,
                              const std::string& value)
{
  mNodeMap.setEnum(parameterName, value);
}

void ConnectedDevice::setString(const std::string& parameterName,
                                const std::string& value)
{
  mNodeMap.setString(parameterName, value, !mRegisterStreamingActive);
}

float ConnectedDevice::getFloatMin(const std::string& parameterName) const
{
  return mNodeMap.getFloatMin(parameterName);
}

float ConnectedDevice::getFloatMax(const std::string& parameterName) const
{
  return mNodeMap.getFloatMax(parameterName);
}

int64_t ConnectedDevice::getIntMin(const std::string& parameterName) const
{
  return mNodeMap.getIntMin(parameterName);
}

int64_t ConnectedDevice::getIntMax(const std::string& parameterName) const
{
  return mNodeMap.getIntMax(parameterName);
}

void ConnectedDevice::exportParameters(std::ostream& outputCsv)
{
  exportDeviceParameters(mNodeMap.get()._Ptr, outputCsv);
}

ConfigurationResult ConnectedDevice::importParameters(std::istream& inputCsv)
{
  return importDeviceParameters(mNodeMap.get()._Ptr, inputCsv);
}

void ConnectedDevice::sendFileToCamera(const std::string& cameraFileName,
                                       std::istream& contents)
{
  std::ostringstream ss;
  ss << contents.rdbuf();
  const std::string str = ss.str();
  GenApi::FileProtocolAdapter adapter;
  geniranger::sendFileContents(
    str, cameraFileName, mNodeMap.get()._Ptr, adapter);
}

std::string
ConnectedDevice::retrieveFileFromCamera(const std::string& cameraFileName)
{
  return geniranger::retrieveFileContents(cameraFileName, mNodeMap.get()._Ptr);
}

void ConnectedDevice::deleteFileFromCamera(const std::string& cameraFileName)
{
  GenApi::FileProtocolAdapter adapter;
  geniranger::deleteFile(cameraFileName, mNodeMap.get()._Ptr, adapter);
}

void ConnectedDevice::updateFirmware(const std::string& firmwarePackagePath,
                                     bool waitForCompletion)
{
  if (!isEnum("FirmwareUpdateProgress") || !isFloat("DeviceTemperature"))
  {
    throw GenIRangerException("The device doesn't have expected parameters, is "
                              "it really a Ranger3 device?");
  }

  uint32_t checksum = Crc32().calculate(firmwarePackagePath);

  geniranger::updateFirmware(firmwarePackagePath, checksum, mNodeMap.get());

  if (waitForCompletion)
  {
    waitForFirmwareUpdateCompletion(getNodeMap());
  }
}

}
