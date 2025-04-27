// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once
#include "../../public/GenIRangerDll.h" // VLB Modified
#include "CApi.h"
#include "GenApi/IPort.h"

#include <TLI/GenTL.h>

#include <memory>
#include <string>

namespace gentlcpp {

class CApi;
class Module;

/**
 * Represents a port to a GenTL module object, or a remote device.
 *
 * Port objects can be retrieved from the corresponding module object, for
 * instance, \ref Interface::getPort.
 *
 * See \ref getGenApiPort for connecting this port to GenApi.
 */
class Port
{
public:
	GENIRANGER_API Port(std::shared_ptr<const CApi> cApi, GenTL::PORT_HANDLE handle); // VLB Modified

    //GENIRANGER_API ~Port() noexcept;
    GENIRANGER_API ~Port() noexcept;

  GenTL::PORT_HANDLE getHandle();
  /**
   * This is a convenience function to be able to feed GenApi C++ classes with
   * an IPort*, e.g., to be able to access a node map.
   */
  GenApi::IPort* getGenApiPort();

  size_t read(uint64_t address, void* pBuffer, size_t size) const;
  size_t write(uint64_t address, const void* pBuffer, size_t size);

  std::string getVendor() const;
  std::string getModel() const;
  std::string getTlType() const;
  std::string getModule() const;
  bool isLittleEndian() const;
  bool isBigEndian() const;
  bool hasReadAccess() const;
  bool hasWriteAccess() const;
  bool isAccessNA() const;
  bool isAccessNI() const;
  std::string getVersion() const;
  std::string getName() const;

  int32_t getUrlCount() const;
  std::string getUrl(uint32_t urlIndex) const;
  int32_t getSchemaVersionMajor(uint32_t urlIndex) const;
  int32_t getSchemaVersionMinor(uint32_t urlIndex) const;
  int32_t getFileVersionMajor(uint32_t urlIndex) const;
  int32_t getFileVersionMinor(uint32_t urlIndex) const;
  int32_t getFileVersionSubminor(uint32_t urlIndex) const;
  /** \return a Sha1 sum of the file as a 20-byte vector. */
  std::vector<uint8_t> getFileSha1Hash(uint32_t urlIndex) const;
  uint64_t getFileRegisterAddress(uint32_t urlIndex) const;
  uint64_t getFileSize(uint32_t urlIndex) const;
  int32_t getScheme(uint32_t urlIndex) const;
  std::string getFilename(uint32_t urlIndex) const;

  size_t getUrlInfoBuffer(uint32_t urlIndex,
                          void* pBuffer,
                          size_t size,
                          GenTL::URL_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(GenTL::PORT_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(value);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(
      mCApi, mCApi->GCGetPortInfo(mHandle, command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::PORT_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(static_cast<GenTL::PORT_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif


  template<typename T>
  T getUrlInfo(uint32_t urlIndex, GenTL::URL_INFO_CMD_LIST command) const
  {
    T value = 0;
    size_t size = sizeof(value);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi,
                 mCApi->GCGetPortURLInfo(
                   mHandle, urlIndex, command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getUrlInfo(uint32_t urlIndex,
                         GenTL::URL_INFO_CMD_LIST command) const;

  template<typename T>
  T getUrlInfo(uint32_t urlIndex, uint32_t customCommand) const
  {
    return getUrlInfo<T>(urlIndex,
                         static_cast<GenTL::URL_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getUrlInfo);
#endif

private:
  /** Adapter for connecting a GenApi NodeMap to a GenTL port */
  class GenApiPort : public GenApi::IPort
  {
  public:
    GenApiPort(Port* parent);
    virtual void Read(void* pBuffer, int64_t address, int64_t length) override;
    virtual void
    Write(const void* pBuffer, int64_t address, int64_t length) override;
    virtual GenApi::EAccessMode GetAccessMode() const override;

  private:
    Port* mParent;
  };

private:
  friend class DataStream;
  friend class Device;
  friend class Interface;
  friend class TransportLayer;

  Port(std::shared_ptr<const CApi> cApi,
       std::shared_ptr<const Module> parent,
       GenTL::PORT_HANDLE handle);
  Port(const Port&) = delete;

private:
  std::shared_ptr<const CApi> mCApi;
  std::shared_ptr<const Module> mParent;
  GenTL::PORT_HANDLE mHandle;
  GenApiPort mGenApiWrapper;
};

}
