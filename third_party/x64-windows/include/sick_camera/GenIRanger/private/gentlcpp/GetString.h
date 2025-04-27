// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include <TLI/GenTL.h>

namespace gentlcpp {

/**
 * Helper macro to call GenTL function that are "returning" string data. If the
 * initially allocated string is not large enough, a sufficiently large one will
 * be allocated.
 */
#define GENTL_GET_STRING(call, ...)                                            \
  do                                                                           \
  {                                                                            \
    char buffer[1024];                                                         \
    size_t bufferSize = sizeof(buffer);                                        \
    GenTL::GC_ERROR status = call(__VA_ARGS__, buffer, &bufferSize);           \
    if (status == GenTL::GC_ERR_BUFFER_TOO_SMALL)                              \
    {                                                                          \
      std::unique_ptr<char[]> largerBuffer(new char[bufferSize]);              \
      gentlcpp::ThrowIfError(                                                  \
        mCApi, call(__VA_ARGS__, largerBuffer.get(), &bufferSize));            \
      return std::string(largerBuffer.get());                                  \
    }                                                                          \
    gentlcpp::ThrowIfError(mCApi, status);                                     \
    return std::string(buffer);                                                \
    __pragma(warning(suppress : 4127))                                         \
  } while (false)

}
