// Copyright 2019-2020 SICK AG. All rights reserved.

#pragma once

#include "FrameGrabber.h"
#include "GenIRangerDataTypes.h"
#include "NodeMap.h"

namespace gentlcpp {
class Device;
class Port;
}

namespace geniranger {

/**
 * A high-level wrapper class for a gentlcpp::Device.
 *
 * When created the wrapper will open the genlcpp::DataStream (a single one is
 * assumed which is true for Ranger3 cameras).
 *
 * The class also allows access to the GenApi node map for the remote device.
 * For convenience there are helper functions to get and set parameter values
 * directly in this class instead of via the node map.
 */
class ConnectedDevice
{
public:
  explicit ConnectedDevice(std::shared_ptr<gentlcpp::Device> device);

  /** Creates a FrameGrabber from this connected device. */
  std::shared_ptr<geniranger::FrameGrabber>
  createFrameGrabber(size_t buffersCount = 20);

  /** \return the underlying \ref gentlcpp::Device object */
  std::shared_ptr<gentlcpp::Device> getDevice();

  /** \return the node map for the remote device */
  NodeMap getNodeMap();

  /** \return the data stream from the device */
  std::shared_ptr<gentlcpp::DataStream> getDataStream();

  /**
   * Starts a register streaming session which is used to atomically set several
   * parameters.
   *
   * Note: PC side verification is automatically turned off when setting
   * parameters through this object. If you want to avoid this you can use the
   * remote device node map available through \ref getNodeMap().
   *
   * \throws GenIRangerException if a streaming session is already started.
   */
  void registerStreamingStart();

  /**
   * Ends a register streaming sessions and makes the device perform
   * reconfiguration accordingly.
   *
   * \throws GenIRangerException if a streaming session is not started.
   * \throws InvalidConfiguration if the configuration was invalid.
   */
  void registerStreamingEnd();

  /** \return the access mode of a device parameter */
  GenApi::EAccessMode getAccessMode(const std::string& parameterName) const;

  /** \return true if the device parameter is a command */
  bool isCommand(const std::string& parameterName) const;

  /** \return true if the device parameter is a boolean */
  bool isBool(const std::string& parameterName) const;

  /** \return true if the device parameter is a float */
  bool isFloat(const std::string& parameterName) const;

  /** \return true if the device parameter is an integer */
  bool isInt(const std::string& parameterName) const;

  /** \return true if the device parameter is an enumeration */
  bool isEnum(const std::string& parameterName) const;

  /** \return true if the device parameter is a string */
  bool isString(const std::string& parameterName) const;

  /** Executes a command parameter on the device */
  void executeCommand(const std::string& parameterName,
                      bool waitForCompletion = true);

  /** \return the value of a boolean device parameter */
  bool getBool(const std::string& parameterName) const;

  /** \return the value of a float device parameter */
  float getFloat(const std::string& parameterName) const;

  /** \return the value of an integer device parameter */
  int64_t getInt(const std::string& parameterName) const;

  /** \return the value of an enumeration device parameter as a string */
  std::string getEnum(const std::string& parameterName) const;
  /** \return the value of an enumeration device parameter as an integer */
  int64_t getEnumInt(const std::string& parameterName) const;

  /** \return the value of a string device parameter */
  std::string getString(const std::string& parameterName) const;

  /** Sets the value of a boolean device parameter */
  void setBool(const std::string& parameterName, bool value);

  /** Sets the value of a float device parameter */
  void setFloat(const std::string& parameterName, float value);

  /** Sets the value of an integer device parameter */
  void setInt(const std::string& parameterName, int64_t value);

  /** Sets the value of an enumeration device parameter */
  void setEnum(const std::string& parameterName, const std::string& value);

  /** Sets the value of a string device parameter */
  void setString(const std::string& parameterName, const std::string& value);

  /** \return the min allowed value for a float device parameter */
  float getFloatMin(const std::string& parameterName) const;
  /** \return the max allowed value for a float device parameter */
  float getFloatMax(const std::string& parameterName) const;

  /** \return the min allowed value for an integer device parameter */
  int64_t getIntMin(const std::string& parameterName) const;
  /** \return the max allowed value for an integer device parameter */
  int64_t getIntMax(const std::string& parameterName) const;

  /**
   * Exports all writable parameters as a CSV file with the format
   * "<name>,<value>", one parameter per row.
   *
   * \see \ref importParameters()
   */
  void exportParameters(std::ostream& outputCsv);

  /**
   * Imports parameters from a CSV file. The format is "<name>,<value>", one
   * parameter per row.
   *
   * \see \ref exportParameters()
   */
  ConfigurationResult importParameters(std::istream& inputCsv);

  /**
   * Sends contents to a camera (device) file. The given camera file corresponds
   * to the value of camera GenICam parameter "FileSelector". File named
   * "UserFile" can be used for arbitrary file storage on camera, but has a size
   * limit of 100,000 bytes.
   *
   * \param cameraFileName the name of the GenICam file on the camera, i.e., the
   *        FileSelector
   * \param contents the contents to write to the camera file
   */
  void sendFileToCamera(const std::string& cameraFileName,
                        std::istream& contents);

  /**
   * Retrieves the contents of a camera (device) file. The given device file
   * corresponds to the value of device GenICam parameter "FileSelector".
   *
   * \param cameraFileName the name of the GenICam file on the camera, i.e., the
   *        FileSelector
   * \return the device file contents
   */
  std::string retrieveFileFromCamera(const std::string& cameraFileName);

  /**
   * Deletes a file from the camera. The given camera file corresponds to the
   * value of camera GenICam parameter "FileSelector".
   *
   * \param cameraFileName the name of the GenICam file on the camera, i.e., the
   *        FileSelector
   */
  void deleteFileFromCamera(const std::string& cameraFileName);

  /**
   * Updates the firmware of the device. This function can wait for the firmware
   * update to complete and the device is being rebooted. The connection to the
   * device is then lost and the user has to reconnect.
   */
  void updateFirmware(const std::string& firmwarePackagePath,
                      bool waitForCompletion = true);

private:
  std::shared_ptr<gentlcpp::Device> mDevice;
  NodeMap mNodeMap;
  std::shared_ptr<gentlcpp::DataStream> mDataStream;
  bool mRegisterStreamingActive;
};

}
