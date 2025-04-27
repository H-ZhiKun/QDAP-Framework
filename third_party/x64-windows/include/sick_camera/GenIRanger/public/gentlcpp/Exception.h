// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once
#include "../../public/GenIRangerDll.h"// VLB Modified

#include <exception>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <string>

#ifndef SWIG
#  include <TLI/GenTL.h>
#endif

namespace gentlcpp {

class CApi;

/**
 * Base class for gentlcpp exceptions. This is used to wrap GenTL error codes
 * into exceptions.
 */
class Exception : public std::exception
{
public:
  virtual ~Exception() noexcept {}

  virtual const char* what() const noexcept override { return mMessage.c_str(); }

protected:
  // GC_ERROR rather than GC_ERROR_LIST to allow custom IDs
  Exception(const std::string& message, GenTL::GC_ERROR id)
  {
    std::ostringstream what;

    what << message;
    if (id != -1)
    {
      what << " (ID: " << id << ")";
    }
    mMessage = what.str();
  }

private:
  std::string mMessage;
};

#define DECLARE_GENTL_EXCEPTION(name, id)                                      \
  class name : public Exception                                                \
  {                                                                            \
  public:                                                                      \
    name(const std::string& message)                                           \
      : Exception(message, id)                                                 \
    {                                                                          \
    }                                                                          \
    virtual ~name() noexcept {}                                                 \
  };

// GenTL errors as exceptions
DECLARE_GENTL_EXCEPTION(ErrorException, GenTL::GC_ERR_ERROR)
DECLARE_GENTL_EXCEPTION(NotInitializedException, GenTL::GC_ERR_NOT_INITIALIZED)
DECLARE_GENTL_EXCEPTION(NotImplementedException, GenTL::GC_ERR_NOT_IMPLEMENTED)
DECLARE_GENTL_EXCEPTION(ResourceInUseException, GenTL::GC_ERR_RESOURCE_IN_USE)
DECLARE_GENTL_EXCEPTION(AccessDeniedException, GenTL::GC_ERR_ACCESS_DENIED)
DECLARE_GENTL_EXCEPTION(InvalidHandleException, GenTL::GC_ERR_INVALID_HANDLE)
DECLARE_GENTL_EXCEPTION(InvalidIdException, GenTL::GC_ERR_INVALID_ID)
DECLARE_GENTL_EXCEPTION(NoDataException, GenTL::GC_ERR_NO_DATA)
DECLARE_GENTL_EXCEPTION(InvalidParameterException,
                        GenTL::GC_ERR_INVALID_PARAMETER)
DECLARE_GENTL_EXCEPTION(IoException, GenTL::GC_ERR_IO)
DECLARE_GENTL_EXCEPTION(TimeoutException, GenTL::GC_ERR_TIMEOUT)
DECLARE_GENTL_EXCEPTION(AbortException, GenTL::GC_ERR_ABORT)
DECLARE_GENTL_EXCEPTION(InvalidBufferException, GenTL::GC_ERR_INVALID_BUFFER)
DECLARE_GENTL_EXCEPTION(NotAvailableException, GenTL::GC_ERR_NOT_AVAILABLE)
DECLARE_GENTL_EXCEPTION(InvalidAddressException, GenTL::GC_ERR_INVALID_ADDRESS)
DECLARE_GENTL_EXCEPTION(BufferTooSmallException, GenTL::GC_ERR_BUFFER_TOO_SMALL)
DECLARE_GENTL_EXCEPTION(InvalidIndexException, GenTL::GC_ERR_INVALID_INDEX)
DECLARE_GENTL_EXCEPTION(ParsingChunkDataException,
                        GenTL::GC_ERR_PARSING_CHUNK_DATA)
DECLARE_GENTL_EXCEPTION(InvalidValueException, GenTL::GC_ERR_INVALID_VALUE)
DECLARE_GENTL_EXCEPTION(ResourceExhaustedException,
                        GenTL::GC_ERR_RESOURCE_EXHAUSTED)
DECLARE_GENTL_EXCEPTION(OutOfMemoryException, GenTL::GC_ERR_OUT_OF_MEMORY)
DECLARE_GENTL_EXCEPTION(BusyException, GenTL::GC_ERR_BUSY)

// Custom exceptions
DECLARE_GENTL_EXCEPTION(LoadLibraryException, GenTL::GC_ERR_CUSTOM_ID - 1)
DECLARE_GENTL_EXCEPTION(ClosedException, GenTL::GC_ERR_CUSTOM_ID - 2)

#undef DECLARE_GENTL_EXCEPTION

class UnknownException : public Exception
{
public:
  UnknownException(const std::string& message, GenTL::GC_ERROR id)
    : Exception(message, id)
  {
  }
};

/**
 * Throws an exception unless status indicates success. If
 * appendLastErrorMessage is true, the details from GCGetLastErrorMessage will
 * be appended to the exception.
 */
GENIRANGER_API 
void ThrowIfError(std::shared_ptr<const CApi> cApi,
                  GenTL::GC_ERROR status,
                  bool appendLastErrorMessage = true);
}
