// Copyright 2019-2020 SICK AG. All rights reserved.

#include "../public/DeviceDiscovery.h"
#include "../public/ConnectedDevice.h"
#include "../public/Exceptions.h"
#include "../public/SharedInterfaces.h"
#include "../public/gentlcpp/CApi.h"
#include "../public/gentlcpp/Device.h"
#include "../public/gentlcpp/DeviceInfo.h"
#include "../public/gentlcpp/Interface.h"
#include "../public/gentlcpp/InterfaceInfo.h"
#include "../public/gentlcpp/Producer.h"

#include <thread>

namespace {

std::vector<std::shared_ptr<gentlcpp::DeviceInfo>>
performDiscovery(std::shared_ptr<gentlcpp::Interface> iface,
                 std::chrono::milliseconds timeout)
{
  iface->updateDeviceList(timeout);
  return iface->getDevices();
}

/**
 * Iterate the interface nodemap until the corresponding device is selected.
 *
 * \throws geniranger::DeviceNodeFoundException if the device is not found
 */
void selectDeviceWithId(geniranger::NodeMap interfaceNodeMap,
                        const std::string& deviceId)
{
  if (GenApi::IsWritable(interfaceNodeMap.getAccessMode("DeviceSelector")))
  {
    int64_t selectorMin = interfaceNodeMap.getIntMin("DeviceSelector");
    int64_t selectorMax = interfaceNodeMap.getIntMax("DeviceSelector");

    for (int64_t index = selectorMin; index <= selectorMax; ++index)
    {
      interfaceNodeMap.setInt("DeviceSelector", index);
      if (interfaceNodeMap.getString("DeviceID") == deviceId)
      {
        return;
      }
    }
  }
  std::ostringstream ss;
  ss << "Found no device with id: " << deviceId;
  std::string message = ss.str();
  throw geniranger::DeviceNotFoundException(message.c_str());
}

}

namespace geniranger {

std::shared_ptr<DeviceDiscovery>
DeviceDiscovery::createFromProducerFile(const std::string& absolutePath)
{
  std::shared_ptr<gentlcpp::Producer> producer =
    gentlcpp::Producer::load(absolutePath);
  std::shared_ptr<gentlcpp::TransportLayer> transportLayer =
    producer->openTransportLayer();
  std::shared_ptr<geniranger::SharedInterfaces> sharedInterfaces =
    std::make_shared<geniranger::SharedInterfaces>(transportLayer);

  return createFromSharedInterfaces(sharedInterfaces);
}

std::shared_ptr<DeviceDiscovery> DeviceDiscovery::createFromSharedInterfaces(
  std::shared_ptr<SharedInterfaces> interfaces)
{
  return std::shared_ptr<DeviceDiscovery>(new DeviceDiscovery(interfaces));
}


std::vector<std::shared_ptr<gentlcpp::DeviceInfo>>
DeviceDiscovery::scanForDevices(std::chrono::milliseconds timeout)
{
  std::vector<std::shared_ptr<gentlcpp::DeviceInfo>> discoveredDevices;
  // No need to specify timeout, since listing network interfaces is
  // synchronous for GigE Vision
  for (auto& interfaceInfo :
       mInterfaces->getInterfaces(std::chrono::milliseconds(0)))
  {
    std::shared_ptr<gentlcpp::Interface> iface =
      mInterfaces->ensureOpen(interfaceInfo->getId());

    // Perform a scan, to find devices on the interface
    for (auto& deviceInfo : performDiscovery(iface, timeout))
    {
      discoveredDevices.push_back(deviceInfo);
    }
  }
  return discoveredDevices;
}

std::shared_ptr<ConnectedDevice>
DeviceDiscovery::openDevice(std::shared_ptr<gentlcpp::DeviceInfo> deviceInfo)
{
  std::shared_ptr<gentlcpp::Device> device =
    deviceInfo->getParent()->openDevice(deviceInfo->getId(),
                                        GenTL::DEVICE_ACCESS_EXCLUSIVE);
  return std::make_shared<ConnectedDevice>(device);
}

std::shared_ptr<ConnectedDevice>
DeviceDiscovery::openDeviceByIpAddress(Ip4Address ipAddress,
                                       std::chrono::milliseconds timeout)
{
  return openFirstMatchingDevice(
    [ipAddress](const gentlcpp::DeviceInfo& deviceInfo) {
      return deviceInfo.getInfo<uint32_t>(SICK_CUSTOM_COMMAND_DEVICE_IP_ADDRESS)
             == ipAddress.value();
    },
    timeout);
}

std::shared_ptr<ConnectedDevice>
DeviceDiscovery::openDeviceByMacAddress(MacAddress macAddress,
                                        std::chrono::milliseconds timeout)
{
  return openFirstMatchingDevice(
    [macAddress](const gentlcpp::DeviceInfo& deviceInfo) {
      return deviceInfo.getInfo<uint64_t>(SICK_CUSTOM_COMMAND_MAC_ADDRESS)
             == macAddress.value();
    },
    timeout);
}

void DeviceDiscovery::reconnect(
  std::shared_ptr<ConnectedDevice>& connectedDevice,
  std::chrono::seconds interval,
  std::chrono::seconds timeout)
{
  // This function can never be completely safe, but let's at least check that
  // the shared_ptr provided holds the only reference to the ConnectedDevice
  // object.
  if (connectedDevice.use_count() > 1)
  {
    throw GenIRangerException(
      "DeviceDiscovery::reconnect() must be called with the only reference to "
      "ConnectedDevice object");
  }

  gentlcpp::DeviceId deviceId = connectedDevice->getDevice()->getId();
  // Destroy the old object
  connectedDevice.reset();

  auto start = std::chrono::system_clock::now();
  do
  {
    auto devices = scanForDevices();
    for (std::shared_ptr<gentlcpp::DeviceInfo> deviceInfo : devices)
    {
      if (deviceInfo->getId() == deviceId)
      {
        std::shared_ptr<gentlcpp::Device> device =
          deviceInfo->getParent()->openDevice(deviceInfo->getId(),
                                              GenTL::DEVICE_ACCESS_EXCLUSIVE);
        connectedDevice.reset(new ConnectedDevice(device));
        return;
      }
    }
    std::this_thread::sleep_for(interval);
  } while (std::chrono::system_clock::now() < start + timeout);
  throw GenIRangerException("Cannot find the device on the network");
}

void DeviceDiscovery::forceIp(std::shared_ptr<gentlcpp::DeviceInfo> device,
                              geniranger::Ip4Address newIpAddress,
                              geniranger::Ip4Address newSubnetMask)
{
  std::shared_ptr<gentlcpp::Interface> iface = device->getParent();

  // Create a NodeMap of the GenTL Producer interface module
  // This allows access to GigEVision specific functionality, such as ForceIP,
  // that is not part of the GenTL C api.
  NodeMap interfaceNodeMap(iface->getPort());
  selectDeviceWithId(interfaceNodeMap, device->getId());

  std::cout << "Changing IP address with ForceIP..." << std::endl;
  interfaceNodeMap.setInt("GevDeviceForceIPAddress", newIpAddress.value());
  interfaceNodeMap.setInt("GevDeviceForceSubnetMask", newSubnetMask.value());

  // Currently, there is no way for us to know when the device has completed the
  // IP address change, nor if it succeeded.
  interfaceNodeMap.executeCommand("GevDeviceForceIP", false);
}

std::shared_ptr<SharedInterfaces> DeviceDiscovery::getInterfaces()
{
  return mInterfaces;
}

DeviceDiscovery::DeviceDiscovery(std::shared_ptr<SharedInterfaces> interfaces)
  : mInterfaces(interfaces)
{
}

/**
 * Connects to the first found device that matches.
 *
 * \throws DeviceNotFoundException if no matching device is found
 */
std::shared_ptr<ConnectedDevice> DeviceDiscovery::openFirstMatchingDevice(
  const std::function<bool(const gentlcpp::DeviceInfo&)> deviceMatcher,
  std::chrono::milliseconds timeout)
{
  // No need to specify timeout, since listing network interfaces is
  // synchronous for GigE Vision
  for (auto& interfaceInfo :
       mInterfaces->getInterfaces(std::chrono::milliseconds(0)))
  {
    std::shared_ptr<gentlcpp::Interface> iface =
      mInterfaces->ensureOpen(interfaceInfo->getId());

    // Perform a scan, to find devices on the interface
    for (auto& deviceInfo : performDiscovery(iface, timeout))
    {
      if (deviceMatcher(*deviceInfo))
      {
        std::shared_ptr<gentlcpp::Device> device = iface->openDevice(
          deviceInfo->getId(), GenTL::DEVICE_ACCESS_EXCLUSIVE);
        return std::make_shared<ConnectedDevice>(device);
      }
    }
  }
  throw DeviceNotFoundException("Found no matching device");
}

}
