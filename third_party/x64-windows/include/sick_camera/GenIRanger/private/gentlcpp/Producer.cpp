// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Producer.h"

#include "GetString.h"
#include "../../public/gentlcpp/CApi.h"
#include "../../public/gentlcpp/Tracing.h"
#include "../../public/gentlcpp/TransportLayer.h"

namespace gentlcpp {

int Producer::msNextInternalId = 0;

std::string Producer::getCompliantGenTlVersion()
{
  return "1.5";
}

std::shared_ptr<Producer> Producer::create(std::shared_ptr<const CApi> cApi)
{
  return std::shared_ptr<Producer>(new Producer(cApi));
}


std::shared_ptr<Producer> Producer::load(const std::string& ctiFilePath)
{
  return create(CApi::load(ctiFilePath));
}

Producer::~Producer()
{
  trace("~Producer(%d)", mInternalId);
  mCApi->GCCloseLib();
}

std::shared_ptr<TransportLayer> Producer::openTransportLayer()
{
  return std::shared_ptr<TransportLayer>(
    new TransportLayer(mCApi, shared_from_this()));
}

std::string Producer::getVendor() const
{
  return getInfo<std::string>(GenTL::TL_INFO_VENDOR);
}

std::string Producer::getModel() const
{
  return getInfo<std::string>(GenTL::TL_INFO_MODEL);
}

std::string Producer::getVersion() const
{
  return getInfo<std::string>(GenTL::TL_INFO_VERSION);
}

std::string Producer::getTlType() const
{
  return getInfo<std::string>(GenTL::TL_INFO_TLTYPE);
}

std::string Producer::getName() const
{
  return getInfo<std::string>(GenTL::TL_INFO_NAME);
}

std::string Producer::getPathname() const
{
  return getInfo<std::string>(GenTL::TL_INFO_PATHNAME);
}

std::string Producer::getDisplayname() const
{
  return getInfo<std::string>(GenTL::TL_INFO_DISPLAYNAME);
}

uint32_t Producer::getGenTlVersionMajor() const
{
  return getInfo<uint32_t>(GenTL::TL_INFO_GENTL_VER_MAJOR);
}

uint32_t Producer::getGenTlVersionMinor() const
{
  return getInfo<uint32_t>(GenTL::TL_INFO_GENTL_VER_MINOR);
}

template<>
std::string Producer::getInfo(GenTL::TL_INFO_CMD_LIST command) const
{
  GenTL::INFO_DATATYPE dataType;
  GENTL_GET_STRING(mCApi->GCGetInfo, command, &dataType);
}

Producer::Producer(std::shared_ptr<const CApi> cApi)
  : mCApi(cApi)
  , mInternalId(msNextInternalId++)
{
  trace("Producer(%d)", mInternalId);

  ThrowIfError(cApi, cApi->GCInitLib());
}

}
