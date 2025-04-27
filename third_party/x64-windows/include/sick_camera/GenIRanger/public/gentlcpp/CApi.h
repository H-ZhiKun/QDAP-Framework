// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

#include <TLI/GenTL.h>

#include <exception>
#include <map>
#include <memory>
#include <sstream>
#define NOMINMAX
#define _WINSOCKAPI_
#include <windows.h>

#include "../GenIRangerDll.h"
#include "Exception.h"

// clang-format off
#define FOR_EACH_GENTL_FUNCTION(code)\
  code(GCGetInfo)\
  code(GCGetLastError)\
  code(GCInitLib)\
  code(GCCloseLib)\
  code(GCReadPort)\
  code(GCWritePort)\
  code(GCGetPortURL)\
  code(GCGetPortInfo)\
  code(GCRegisterEvent)\
  code(GCUnregisterEvent)\
  code(EventGetData)\
  code(EventGetDataInfo)\
  code(EventGetInfo)\
  code(EventFlush)\
  code(EventKill)\
  code(TLOpen)\
  code(TLClose)\
  code(TLGetInfo)\
  code(TLGetNumInterfaces)\
  code(TLGetInterfaceID)\
  code(TLGetInterfaceInfo)\
  code(TLOpenInterface)\
  code(TLUpdateInterfaceList)\
  code(IFClose)\
  code(IFGetInfo)\
  code(IFGetNumDevices)\
  code(IFGetDeviceID)\
  code(IFUpdateDeviceList)\
  code(IFGetDeviceInfo)\
  code(IFOpenDevice)\
  code(DevGetPort)\
  code(DevGetNumDataStreams)\
  code(DevGetDataStreamID)\
  code(DevOpenDataStream)\
  code(DevGetInfo)\
  code(DevClose)\
  code(DSAnnounceBuffer)\
  code(DSAllocAndAnnounceBuffer)\
  code(DSFlushQueue)\
  code(DSStartAcquisition)\
  code(DSStopAcquisition)\
  code(DSGetInfo)\
  code(DSGetBufferID)\
  code(DSClose)\
  code(DSRevokeBuffer)\
  code(DSQueueBuffer)\
  code(DSGetBufferInfo)\
  code(GCGetNumPortURLs)\
  code(GCGetPortURLInfo)\
  code(GCReadPortStacked)\
  code(GCWritePortStacked)\
  code(DSGetBufferChunkData)\
  code(IFGetParentTL)\
  code(DevGetParentIF)\
  code(DSGetParentDev)\
  code(DSGetNumBufferParts)\
  code(DSGetBufferPartInfo)
// clang-format on

namespace gentlcpp {

/**
 * This is a collection of function pointers to the GenTL C API. When the CTI
 * file is loaded the pointers will point to the function in the library.
 */
class GENIRANGER_API CApi
{
public:
  static std::shared_ptr<const CApi> load(const std::string& ctiFile);
  CApi(const CApi* p) {
	  this->mModule = p->mModule;
  };
  //~CApi() noexcept;
  ~CApi() noexcept;

#define FUNC_PTR(func) GenTL::P##func func;
  FOR_EACH_GENTL_FUNCTION(FUNC_PTR)
#undef FUNC_PTR

private:
  CApi(HMODULE module);
  CApi(const CApi&) = delete;

private:
  HMODULE mModule;
};

}

// Macro for checking if a call to GenTL succeeded
#define CC(cApi, CALL) gentlcpp::ThrowIfError(cApi, (CALL));
