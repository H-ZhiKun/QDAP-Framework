// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once
#include "GenIRangerDll.h"// VLB Modified

#include <cstdint>
#include <memory>
#include <string>

namespace gentlcpp {
class Device;
class DeviceInfo;
}

namespace geniranger {

/** A representation of an Ethernet MAC address. */
class MacAddress
{
public:
  /**
   * \param address the address as an integer in little endian.
   * \throw InvalidMacAddress if the value is too large.
   */
  explicit MacAddress(uint64_t address);
  /**
   * \param address the address as colon separated string.
   * \throw InvalidMacAddress if the format is invalid.
   */
  explicit MacAddress(const std::string& address);

  /** Create an MacAddress object with the MAC address of the device. */
  static MacAddress fromDevice(std::shared_ptr<gentlcpp::Device> device);
  /** Create an MacAddress object with the MAC address of the device. */
  static MacAddress
  fromDeviceInfo(std::shared_ptr<gentlcpp::DeviceInfo> device);

  /** \return the MAC address as an integer on little endian. */
  uint64_t value() const;

  /** \return the MAC address as colon separated string. */
  std::string toString() const;

  /**
  * Convert MAC address to std::string. \n
  * \n
  * Add by VisionLab, Shanghai, China.
  */
  GENIRANGER_API static std::string to_String_VLB(const uint64_t & address);

private:
  uint64_t mAddress;
};

}
