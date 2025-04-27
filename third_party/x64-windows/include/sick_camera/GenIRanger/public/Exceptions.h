// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

#include "GenIRangerDll.h"

#include <exception>
#include <string>

namespace geniranger {
/**
 * Generic GenIRanger Exception class. Thrown whenever an error occurs either in
 * GenIRanger or GenICam. The messages from GenICam exceptions are wrapped into
 * GenIRanger Exceptions.
 */
class GENIRANGER_API GenIRangerException : public std::exception
{
public:
  GenIRangerException(const std::string& message);
};

/**
 * Indicates that something went wrong during export configuration from device.
 */
class ExportException : public GenIRangerException
{
public:
  ExportException(const std::string& message);
};

/** Indicates that something went wrong while writing device log to disk. */
class DeviceLogException : public GenIRangerException
{
public:
  DeviceLogException(const std::string& message);
};

/** Indicates that there is some problem when saving data to disk. */
class SaveException : public GenIRangerException
{
public:
  SaveException(const std::string& message);
};

/** Error when accessing an Xml attribute. */
class GENIRANGER_API XmlAttributeException : public GenIRangerException
{
public:
  XmlAttributeException(const std::string& message);
};

/** Indicates that a resource or object is not available. */
class NotAvailableException : public GenIRangerException
{
public:
  NotAvailableException(const std::string& message);
};

/** Indicates that an IP address is invalid. */
class InvalidIpAddress : public GenIRangerException
{
public:
  InvalidIpAddress(const std::string& message);
};

/** Indicates that a MAC address is invalid. */
class InvalidMacAddress : public GenIRangerException
{
public:
  InvalidMacAddress(const std::string& message);
};

/**
 * Indicates that there was some kind of problem with the chunk layout or that
 * the format was unexpected for a Ranger3 camera.
 */
class ChunkFormatException : public GenIRangerException
{
public:
  ChunkFormatException(const std::string& message);
};

/**
 * Indicates that the configuration of the device is invalid after a register
 * streaming session
 */
class InvalidConfiguration : public GenIRangerException
{
public:
  InvalidConfiguration(const std::string& message);
};

}
