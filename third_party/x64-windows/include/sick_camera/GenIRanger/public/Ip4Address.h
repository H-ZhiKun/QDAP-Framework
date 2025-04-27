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

/** A representation of an IP v4 address. */
class Ip4Address
{
public:
  /** \param address the address as an integer in host byte order. */
  explicit Ip4Address(uint32_t address);
  /**
   * \param address the address as dot separated string.
   * \throw InvalidIpAddress if the format is invalid.
   */
  explicit Ip4Address(const std::string& address);

  /** Create an Ip4Address object with the IP address of the device. */
  static Ip4Address fromDevice(std::shared_ptr<gentlcpp::Device> device);
  /** Create an Ip4Address object with the IP address of the device. */
  static Ip4Address
  fromDeviceInfo(std::shared_ptr<gentlcpp::DeviceInfo> device);

  /** \return the IP address as an integer in host byte order. */
  uint32_t value() const;

  /** \return the IP address as dot separated string. */
  std::string toString() const;


  /**
  * Convert IP address to std::string. \n
  * \n
  * Add by VisionLab, Shanghai, China.
  */
  GENIRANGER_API static std::string	to_String_VLB	(const uint32_t & address);

  /**
  * Convert IP address to uint32_t. \n
  * \n
  * Add by VisionLab, Shanghai, China.
  */
  GENIRANGER_API static uint32_t	to_uint32_t_VLB	(const std::string & address);


private:
  uint32_t mAddress;
};

}
