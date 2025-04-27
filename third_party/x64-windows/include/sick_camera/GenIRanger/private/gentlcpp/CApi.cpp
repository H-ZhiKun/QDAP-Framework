// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/CApi.h"

#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

namespace gentlcpp {

std::shared_ptr<const CApi> CApi::load(const std::string& ctiFile)
{
  HMODULE module = LoadLibrary(ctiFile.c_str());
  if (module == nullptr)
  {
    std::ostringstream sstr;
    sstr << "Could not load: " << ctiFile;
    std::string errorMessage(sstr.str());
    std::cerr << errorMessage << std::endl;
    throw std::exception(errorMessage.c_str());
  }
  std::shared_ptr<CApi> cApi(new CApi(module));

#define LOAD_PROC_ADDRESS(func)                                                \
  cApi->func = (GenTL::P##func)GetProcAddress(module, #func);                  \
  assert(cApi->func);

  FOR_EACH_GENTL_FUNCTION(LOAD_PROC_ADDRESS)
#undef LOAD_PROC_ADDRESS
  return cApi;
}

CApi::~CApi()
{
  if (!FreeLibrary(mModule))
  {
    std::cerr << "Could not free cti library" << std::endl;
  }
}

CApi::CApi(HMODULE module)
  : mModule(module)
{
}

}
