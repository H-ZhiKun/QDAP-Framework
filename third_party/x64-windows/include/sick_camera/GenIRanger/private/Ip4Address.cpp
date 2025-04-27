// Copyright 2019-2020 SICK AG. All rights reserved.
#include "../public/Ip4Address.h"

#include "../public/DeviceDiscovery.h"
#include "../public/Exceptions.h"
#include "../public/gentlcpp/CApi.h"
#include "../public/gentlcpp/Device.h"
#include "../public/gentlcpp/DeviceInfo.h"
#include "../public/gentlcpp/Interface.h"

#include <sstream>
#include <ws2tcpip.h>

namespace geniranger {

Ip4Address::Ip4Address(uint32_t address)
  : mAddress(address)
{
  // Empty
}

Ip4Address::Ip4Address(const std::string& address)
{
  uint32_t ipAddress;
  int result = inet_pton(AF_INET, address.c_str(), &ipAddress);
  if (result != 1)
  {
    std::ostringstream ss;
    ss << "Invalid IP address: \"" << address << "\"";
    std::string message = ss.str();
    throw InvalidIpAddress(message);
  }
  mAddress = ntohl(ipAddress);
}

Ip4Address Ip4Address::fromDevice(std::shared_ptr<gentlcpp::Device> device)
{
  return Ip4Address(device->getInfo<uint32_t>(
    DeviceDiscovery::SICK_CUSTOM_COMMAND_DEVICE_IP_ADDRESS));
}

Ip4Address
Ip4Address::fromDeviceInfo(std::shared_ptr<gentlcpp::DeviceInfo> device)
{
  return Ip4Address(device->getInfo<uint32_t>(
    DeviceDiscovery::SICK_CUSTOM_COMMAND_DEVICE_IP_ADDRESS));
}

uint32_t Ip4Address::value() const
{
  return mAddress;
}

std::string Ip4Address::toString() const
{
  char str[INET_ADDRSTRLEN];
  uint32_t addressNetworkByteOrder = htonl(mAddress);
  inet_ntop(AF_INET, &addressNetworkByteOrder, str, sizeof(str));
  return str;
}

//////////////////////////////////////////////////////

#pragma region VisionLab_Modified

std::string Ip4Address::to_String_VLB(const uint32_t & address)
{
	char str[INET_ADDRSTRLEN];
	uint32_t addressNetworkByteOrder = htonl(address);
	InetNtop(AF_INET, &addressNetworkByteOrder, str, sizeof(str));
	return str;
}

uint32_t Ip4Address::to_uint32_t_VLB(const std::string & address)
{
	uint32_t ipAddress;
	int result = InetPton(AF_INET, address.c_str(), &ipAddress);
	if (result != 1)
	{
		std::ostringstream ss;
		ss << "Invalid IP address: \"" << address << "\"";
		std::string message = ss.str();
		throw InvalidIpAddress(message);
	}

	return ntohl(ipAddress);
}

#pragma endregion


}
