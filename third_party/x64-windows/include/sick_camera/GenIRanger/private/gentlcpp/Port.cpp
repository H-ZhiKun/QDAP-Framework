// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Port.h"
#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Device.h"
#include "../../public/gentlcpp/Tracing.h"

#include <iostream>

namespace gentlcpp {

Port::Port(std::shared_ptr<const CApi> cApi, GenTL::PORT_HANDLE handle)// VLB Modified
	: mCApi(cApi)
	, mHandle(handle)
	, mGenApiWrapper(this)
{
	trace("Port() without module");
}

Port::~Port()
{
  trace("~Port()");
}

GenTL::PORT_HANDLE Port::getHandle()
{
  return mHandle;
}

GenApi::IPort* Port::getGenApiPort()
{
  return &mGenApiWrapper;
}

size_t Port::read(uint64_t address, void* pBuffer, size_t size) const
{
  ThrowIfError(mCApi, mCApi->GCReadPort(mHandle, address, pBuffer, &size));
  return size;
}

size_t Port::write(uint64_t address, const void* pBuffer, size_t size)
{
  ThrowIfError(mCApi, mCApi->GCWritePort(mHandle, address, pBuffer, &size));
  return size;
}

std::string Port::getVendor() const
{
  return getInfo<std::string>(GenTL::PORT_INFO_VENDOR);
}

std::string Port::getModel() const
{
  return getInfo<std::string>(GenTL::PORT_INFO_MODEL);
}

std::string Port::getTlType() const
{
  return getInfo<std::string>(GenTL::PORT_INFO_TLTYPE);
}

std::string Port::getModule() const
{
  return getInfo<std::string>(GenTL::PORT_INFO_MODULE);
}

bool Port::isLittleEndian() const
{
  return getInfo<bool8_t>(GenTL::PORT_INFO_LITTLE_ENDIAN);
}

bool Port::isBigEndian() const
{
  return getInfo<bool8_t>(GenTL::PORT_INFO_BIG_ENDIAN);
}

bool Port::hasReadAccess() const
{
  return getInfo<bool8_t>(GenTL::PORT_INFO_ACCESS_READ);
}

bool Port::hasWriteAccess() const
{
  return getInfo<bool8_t>(GenTL::PORT_INFO_ACCESS_WRITE);
}

bool Port::isAccessNA() const
{
  return getInfo<bool8_t>(GenTL::PORT_INFO_ACCESS_NA);
}

bool Port::isAccessNI() const
{
  return getInfo<bool8_t>(GenTL::PORT_INFO_ACCESS_NI);
}

std::string Port::getVersion() const
{
  return getInfo<std::string>(GenTL::PORT_INFO_VERSION);
}

std::string Port::getName() const
{
  return getInfo<std::string>(GenTL::PORT_INFO_PORTNAME);
}

int32_t Port::getUrlCount() const
{
  uint32_t count = 0;
  ThrowIfError(mCApi, mCApi->GCGetNumPortURLs(mHandle, &count));
  return count;
}

std::string Port::getUrl(uint32_t urlIndex) const
{
  return getUrlInfo<std::string>(urlIndex, GenTL::URL_INFO_URL);
}

int32_t Port::getSchemaVersionMajor(uint32_t urlIndex) const
{
  return getUrlInfo<int32_t>(urlIndex, GenTL::URL_INFO_SCHEMA_VER_MAJOR);
}

int32_t Port::getSchemaVersionMinor(uint32_t urlIndex) const
{
  return getUrlInfo<int32_t>(urlIndex, GenTL::URL_INFO_SCHEMA_VER_MINOR);
}

int32_t Port::getFileVersionMajor(uint32_t urlIndex) const
{
  return getUrlInfo<int32_t>(urlIndex, GenTL::URL_INFO_FILE_VER_MAJOR);
}

int32_t Port::getFileVersionMinor(uint32_t urlIndex) const
{
  return getUrlInfo<int32_t>(urlIndex, GenTL::URL_INFO_FILE_VER_MINOR);
}

int32_t Port::getFileVersionSubminor(uint32_t urlIndex) const
{
  return getUrlInfo<int32_t>(urlIndex, GenTL::URL_INFO_FILE_VER_SUBMINOR);
}

std::vector<uint8_t> Port::getFileSha1Hash(uint32_t urlIndex) const
{
  std::vector<uint8_t> hash;
  size_t hashSize = 160 / sizeof(uint8_t);
  hash.resize(hashSize);
  getUrlInfoBuffer(
    urlIndex, hash.data(), hashSize, GenTL::URL_INFO_FILE_SHA1_HASH);
  return hash;
}

uint64_t Port::getFileRegisterAddress(uint32_t urlIndex) const
{
  return getUrlInfo<uint64_t>(urlIndex, GenTL::URL_INFO_FILE_REGISTER_ADDRESS);
}

uint64_t Port::getFileSize(uint32_t urlIndex) const
{
  return getUrlInfo<uint64_t>(urlIndex, GenTL::URL_INFO_FILE_SIZE);
}

int32_t Port::getScheme(uint32_t urlIndex) const
{
  return getUrlInfo<int32_t>(urlIndex, GenTL::URL_INFO_SCHEME);
}

std::string Port::getFilename(uint32_t urlIndex) const
{
  return getUrlInfo<std::string>(urlIndex, GenTL::URL_INFO_FILENAME);
}

size_t Port::getUrlInfoBuffer(uint32_t urlIndex,
                              void* pBuffer,
                              size_t size,
                              GenTL::URL_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  ThrowIfError(mCApi,
               mCApi->GCGetPortURLInfo(
                 mHandle, urlIndex, command, &dataType, pBuffer, &size));
  return size;
}

template<>
std::string Port::getInfo(GenTL::PORT_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->GCGetPortInfo, mHandle, command, &dataType);
}

template<>
std::string Port::getUrlInfo(uint32_t urlIndex,
                             GenTL::URL_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(
    mCApi->GCGetPortURLInfo, mHandle, urlIndex, command, &dataType);
}



Port::GenApiPort::GenApiPort(Port* parent)
  : mParent(parent)
{
}

void Port::GenApiPort::Read(void* pBuffer, int64_t address, int64_t length)
{
  mParent->read(address, pBuffer, length);
}


void Port::GenApiPort::Write(const void* pBuffer,
                             int64_t address,
                             int64_t length)
{
  mParent->write(address, pBuffer, length);
}

GenApi::EAccessMode Port::GenApiPort::GetAccessMode() const
{
  return GenApi::RW;
}

Port::Port(std::shared_ptr<const CApi> cApi,
           std::shared_ptr<const Module> parent,
           GenTL::PORT_HANDLE handle)
  : mCApi(cApi)
  , mParent(parent)
  , mHandle(handle)
  , mGenApiWrapper(this)
{
  trace("Port()");
}

}
