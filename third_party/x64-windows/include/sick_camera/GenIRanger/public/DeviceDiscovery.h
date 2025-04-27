// Copyright 2019-2020 SICK AG. All rights reserved.

#pragma once

#include "Avoid.h"
#include "Ip4Address.h"
#include "MacAddress.h"

#include <chrono>
#include <functional>
#include <memory>
#include <vector>

namespace gentlcpp {
class DeviceInfo;
}

namespace geniranger {

class ConnectedDevice;
class SharedInterfaces;

/** Exception indicating that the specified device could not be found */
class DeviceNotFoundException : public std::exception
{
public:
  explicit DeviceNotFoundException(const char* message)
    : std::exception(message)
  {
  }
};


/**
 * Helper class to handle device discovery and connection to devices via MAC or
 * IP address. Use \ref createFromProducerFile to create an instance.
 */
class DeviceDiscovery
{
public:
  static const uint32_t SICK_CUSTOM_COMMAND_DEVICE_IP_ADDRESS = 8801;
  static const uint32_t SICK_CUSTOM_COMMAND_DEVICE_SUBNET_MASK = 8802;
  static const uint32_t SICK_CUSTOM_COMMAND_MAC_ADDRESS = 8803;

public:
  /**
   * Creates an instance of DeviceDiscovery given the path to a GenTL producer
   * library file (.cti)
   */
  static std::shared_ptr<DeviceDiscovery>
  createFromProducerFile(const std::string& absolutePath);

  /**
   * Creates an instance of DeviceDiscovery given a \ref SharedInterfaces object
   */
  static std::shared_ptr<DeviceDiscovery>
  createFromSharedInterfaces(std::shared_ptr<SharedInterfaces> interfaces);

  /**
   * Scans all network interfaces and returns information on found devices.
   *
   * \param timeout timeout per network interface to scan
   */
  std::vector<std::shared_ptr<gentlcpp::DeviceInfo>> scanForDevices(
    std::chrono::milliseconds timeout = std::chrono::milliseconds(500));

  /**
   * Connects to a device given an information object, e.g., received from \ref
   * scanForDevices.
   */
  static std::shared_ptr<ConnectedDevice>
  openDevice(std::shared_ptr<gentlcpp::DeviceInfo> deviceInfo);

  /**
   * Connects to a device by IP address
   *
   * \param ipAddress an IPv4 address, for example 192.168.0.1
   * \param timeout timeout per network interface to scan
   */
  std::shared_ptr<ConnectedDevice> openDeviceByIpAddress(
    Ip4Address ipAddress,
    std::chrono::milliseconds timeout = std::chrono::milliseconds(500));

  /**
   * Connects to a device by a mac address
   *
   * \param macAddress MAC address of the device to connect to
   * \param timeout timeout per network interface to scan
   */
  std::shared_ptr<ConnectedDevice> openDeviceByMacAddress(
    MacAddress macAddress,
    std::chrono::milliseconds timeout = std::chrono::milliseconds(500));

  /**
   * This function can be used to reconnect to a device when it is known to be
   * rebooting, e.g., after a firmware update.
   *
   * \note This function is dangerous and will close the ConnectedDevice object.
   *       It must be called with the only shared_ptr to the object. If there
   *       are others, they must be reset first.
   * \param[in, out] device the object for a device that we lost connection to.
   *            The smart pointer will be updated to point to a new object.
   * \param interval the time between each attempt to reconnect
   * \param timeout the maximum amount of time to try to reconnect
   * \throw GenIRangerException if the device is not found within the timeout
   */
  void reconnect(std::shared_ptr<ConnectedDevice>& device,
                 std::chrono::seconds interval = std::chrono::seconds(5),
                 std::chrono::seconds timeout = std::chrono::seconds(60));

  /**
   * Temporarily sets the IP address of a device.
   *
   * This can be used to move an unreachable device to another subnet, so that
   * connection is possible.
   *
   * In general, this is only a temporary change of IP address. In order to make
   * the new IP address persistent, one also has to configure the device
   * parameter `GevPersistentIPAddress`.
   *
   * Note: After using this method, one must re-scan to find the device on the
   * new IP address.
   *
   * \param device the device for which to set IP address
   * \param newIpAddress the new IP address to set
   * \param newSubnetMask the new subnet mask to set
   */
  static void forceIp(std::shared_ptr<gentlcpp::DeviceInfo> device,
                      geniranger::Ip4Address newIpAddress,
                      geniranger::Ip4Address newSubnetMask);

  /**
   * \return the underlying \ref SharedInterfaces
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID std::shared_ptr<SharedInterfaces> getInterfaces();

private:
  explicit DeviceDiscovery(std::shared_ptr<SharedInterfaces> interfaces);

  DeviceDiscovery& operator=(const DeviceDiscovery&) = delete;
  DeviceDiscovery(const DeviceDiscovery&) = delete;

  std::shared_ptr<ConnectedDevice> openFirstMatchingDevice(
    std::function<bool(const gentlcpp::DeviceInfo&)> deviceMatcher,
    std::chrono::milliseconds timeout);

private:
  std::shared_ptr<SharedInterfaces> mInterfaces;
};

}
