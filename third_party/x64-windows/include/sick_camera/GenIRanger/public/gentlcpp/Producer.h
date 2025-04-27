// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once
#include "../../public/GenIRangerDll.h" // VLB Modified
#include "CApi.h"
#include <TLI/GenTL.h>

#include <memory>
#include <string>

namespace gentlcpp {

class CApi;
class TransportLayer;

/**
 * Entry point to the object oriented bindings for GenTL. This class takes care
 * of loading the .cti-file and library initialization.
 */
#ifdef SWIG
class Producer
#else
class Producer : public std::enable_shared_from_this<Producer>
#endif
{
public:
  static std::string getCompliantGenTlVersion();
  static std::shared_ptr<Producer> create(std::shared_ptr<const CApi> cApi);
  GENIRANGER_API static std::shared_ptr<Producer> load(const std::string& ctiFilePath);

  ~Producer() noexcept;

  /** Opens the GenTL transport layer */
  std::shared_ptr<TransportLayer> openTransportLayer();

  std::string getVendor() const;
  std::string getModel() const;
  std::string getVersion() const;
  std::string getTlType() const;
  std::string getName() const;
  std::string getPathname() const;
  std::string getDisplayname() const;
  uint32_t getGenTlVersionMajor() const;
  uint32_t getGenTlVersionMinor() const;

  GENIRANGER_API std::shared_ptr<const CApi> tl() { return mCApi; }; // return tl pointer, add by VLB.


  template<typename T>
  T getInfo(GenTL::TL_INFO_CMD_LIST command) const
  {
    T value;
    size_t size = sizeof(value);
    GenTL::INFO_DATATYPE dataType;
    ThrowIfError(mCApi, mCApi->GCGetInfo(command, &dataType, &value, &size));
    return value;
  }

  template<>
  std::string getInfo(GenTL::TL_INFO_CMD_LIST command) const;

  template<typename T>
  T getInfo(uint32_t customCommand) const
  {
    return getInfo<T>(static_cast<GenTL::TL_INFO_CMD_LIST>(customCommand));
  }

#ifdef SWIG
    EXPLODE_TEMPLATE_TYPES(getInfo);
#endif

private:
  Producer(const Producer&) = delete;
  Producer(std::shared_ptr<const CApi> cApi);

  std::shared_ptr<const CApi> mCApi;

  // Debugging id to be able to track object allocations more easily
  int mInternalId;

  static int msNextInternalId;
};

}
