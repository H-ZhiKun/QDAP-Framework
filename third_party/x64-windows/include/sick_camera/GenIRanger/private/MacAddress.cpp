// Copyright 2019-2020 SICK AG. All rights reserved.
#include "../public/MacAddress.h"

#include "../public/DeviceDiscovery.h"
#include "../public/Exceptions.h"
#include "../public/gentlcpp/CApi.h"
#include "../public/gentlcpp/Device.h"
#include "../public/gentlcpp/DeviceInfo.h"
#include "../public/gentlcpp/Interface.h"

#include <iomanip>
#include <sstream>

namespace geniranger {

MacAddress::MacAddress(uint64_t address)
  : mAddress(address)
{
  if (mAddress > 0xffffffffffffULL)
  {
    throw InvalidMacAddress("MAC address too large");
  }
}

MacAddress::MacAddress(const std::string& address)
{
  int a, b, c, d, e, f;
  // To make sure there is nothing after the last part of the MAC
  char _;

  const char* addr = address.c_str();
#pragma warning(suppress : 4996)
  int groups = sscanf(addr, "%x:%x:%x:%x:%x:%x%c", &a, &b, &c, &d, &e, &f, &_);

  // Verify that no group is larger than 0xff
  int sanityCheck = (a | b | c | d | e | f);
  bool anyGroupTooLarge = sanityCheck != (sanityCheck & 0xff);

  if (groups != 6 || anyGroupTooLarge)
  {
    std::ostringstream ss;
    ss << "Invalid MAC address: \"" << address << "\"";
    std::string message = ss.str();
    throw InvalidMacAddress(message);
  }

  uint64_t mac = static_cast<uint64_t>(a) << 40;
  mac = (static_cast<uint64_t>(b) << 32) | mac;
  mac = (static_cast<uint64_t>(c) << 24) | mac;
  mac = (static_cast<uint64_t>(d) << 16) | mac;
  mac = (static_cast<uint64_t>(e) << 8) | mac;
  mac = (static_cast<uint64_t>(f) << 0) | mac;
  mAddress = mac;
}

MacAddress MacAddress::fromDevice(std::shared_ptr<gentlcpp::Device> device)
{
  return MacAddress(device->getInfo<uint64_t>(
    DeviceDiscovery::SICK_CUSTOM_COMMAND_MAC_ADDRESS));
}

MacAddress
MacAddress::fromDeviceInfo(std::shared_ptr<gentlcpp::DeviceInfo> device)
{
  return MacAddress(device->getInfo<uint64_t>(
    DeviceDiscovery::SICK_CUSTOM_COMMAND_MAC_ADDRESS));
}

uint64_t MacAddress::value() const
{
  return mAddress;
}

std::string MacAddress::toString() const
{
  std::ostringstream ss;
  const size_t bytesInMac = 6;
  for (size_t i = 0; i < bytesInMac; ++i)
  {
    ss << std::setfill('0') << std::setw(2) << std::hex
       << ((mAddress >> (bytesInMac - i - 1) * 8) & 0xff);
    if (i != bytesInMac - 1)
    {
      ss << ":";
    }
  }
  return ss.str();
}


//////////////////////////////////////////////////////

#pragma region VisionLab_Modified

std::string MacAddress::to_String_VLB(const uint64_t & address)
{
	if (address > 0xffffffffffffULL)
	{
		throw InvalidMacAddress("MAC address too large");
	}

	std::ostringstream ss;
	const size_t bytesInMac = 6;
	for (size_t i = 0; i < bytesInMac; ++i)
	{
		ss << std::setfill('0') << std::setw(2) << std::hex
			<< ((address >> (bytesInMac - i - 1) * 8) & 0xff);
		if (i != bytesInMac - 1)
		{
			ss << ":";
		}
	}
	return ss.str();
}

#pragma endregion


}
