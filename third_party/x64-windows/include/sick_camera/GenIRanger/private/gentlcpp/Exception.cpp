// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Exception.h"

#include "../../public/gentlcpp/CApi.h"

#include <memory>

#define THROW_EXCEPTION(Exception, BaseMessage, AdditionalMessage)             \
  throw Exception(std::string(BaseMessage)                                     \
                  + std::string(" (Message from the source: ")                 \
                  + AdditionalMessage + std::string(")"));

namespace gentlcpp {

using namespace GenTL;

void ThrowIfError(std::shared_ptr<const CApi> cApi,
                  GC_ERROR status,
                  bool appendLastErrorMessage)
{
  std::string message;
  if (status != GC_ERR_SUCCESS && appendLastErrorMessage)
  {
    char errorText[1024];
    size_t textLength = sizeof(errorText);
    GC_ERROR errorCode;
    GC_ERROR lastErrorStatus =
      cApi->GCGetLastError(&errorCode, errorText, &textLength);
    if (lastErrorStatus == GC_ERR_BUFFER_TOO_SMALL)
    {
      std::unique_ptr<char[]> longerErrorText(new char[textLength]);
      // Pass false, to avoid infinite recursion, since there was an error
      // getting the error.
      ThrowIfError(
        cApi,
        cApi->GCGetLastError(&errorCode, longerErrorText.get(), &textLength),
        false);
      message = std::string(longerErrorText.get());
    }
    else
    {
      ThrowIfError(cApi, status, false);
      message = std::string(errorText);
    }
  }
  // Throw an appropriate exception if needed.
  switch (status)
  {
  case GC_ERR_SUCCESS: break;
  case GC_ERR_ERROR:
    THROW_EXCEPTION(ErrorException, "Unspecified runtime exception.", message);
  case GC_ERR_NOT_INITIALIZED:
    THROW_EXCEPTION(
      NotInitializedException, "Module or resource not initialized.", message);
  case GC_ERR_NOT_IMPLEMENTED:
    THROW_EXCEPTION(
      NotImplementedException, "Requested operation not implemented.", message);
  case GC_ERR_RESOURCE_IN_USE:
    THROW_EXCEPTION(
      ResourceInUseException, "Requested resource is already in use.", message);
  case GC_ERR_ACCESS_DENIED:
    THROW_EXCEPTION(
      AccessDeniedException, "Requested operation is not allowed.", message);
  case GC_ERR_INVALID_HANDLE:
    THROW_EXCEPTION(InvalidHandleException,
                    "Given handle does not support the operation.",
                    message);
  case GC_ERR_INVALID_ID:
    THROW_EXCEPTION(
      InvalidIdException, "ID could not be connected to a resource.", message);
  case GC_ERR_NO_DATA:
    THROW_EXCEPTION(
      NoDataException, "Function has no data to work on.", message);
  case GC_ERR_INVALID_PARAMETER:
    THROW_EXCEPTION(InvalidParameterException,
                    "Parameter not valid or out of range.",
                    message);
  case GC_ERR_IO:
    THROW_EXCEPTION(
      IoException,
      "Communication error, read or write to remote device failed.",
      message);
  case GC_ERR_TIMEOUT:
    THROW_EXCEPTION(
      TimeoutException, "Operation timed out before completion.", message);
  case GC_ERR_ABORT:
    THROW_EXCEPTION(AbortException, "Operation aborted.", message);
  case GC_ERR_INVALID_BUFFER:
    THROW_EXCEPTION(
      InvalidBufferException,
      "Insufficient amount of announced buffers to start acquisition.",
      message);
  case GC_ERR_NOT_AVAILABLE:
    THROW_EXCEPTION(NotAvailableException,
                    "Resource or information is not available.",
                    message);
  case GC_ERR_INVALID_ADDRESS:
    THROW_EXCEPTION(
      InvalidAddressException, "Address is out of range or invalid.", message);
  case GC_ERR_BUFFER_TOO_SMALL:
    THROW_EXCEPTION(BufferTooSmallException,
                    "Buffer used is too small for operation.",
                    message);
  case GC_ERR_INVALID_INDEX:
    THROW_EXCEPTION(
      InvalidIndexException, "Provided index is out of range.", message);
  case GC_ERR_PARSING_CHUNK_DATA:
    THROW_EXCEPTION(
      ParsingChunkDataException, "Error parsing chunk data buffer.", message);
  case GC_ERR_INVALID_VALUE:
    THROW_EXCEPTION(InvalidValueException,
                    "Attempting to write an invalid register value.",
                    message);
  case GC_ERR_RESOURCE_EXHAUSTED:
    THROW_EXCEPTION(ResourceExhaustedException,
                    "The requested resource is exhausted.",
                    message);
  case GC_ERR_OUT_OF_MEMORY:
    THROW_EXCEPTION(OutOfMemoryException,
                    "System and/or hardware has run out of memory.",
                    message);
  case GC_ERR_BUSY:
    THROW_EXCEPTION(
      BusyException,
      "Operation cannot be performed. Responsible module is busy.",
      message);
  default:
    throw UnknownException(
      std::string("Unknown runtime exception. (Message from the source: ")
        + message + ")",
      status);
  }
}

}
