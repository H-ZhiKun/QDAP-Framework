// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once
#include "CApi.h"

#include <string>

namespace gentlcpp {

typedef std::string InterfaceId;
typedef std::string DeviceId;
typedef std::string DataStreamId;

/** Ease-of-use type for the optional user pointer for buffer handling */
typedef void* BufferUserData;

/** Buffer to data within a buffer */
typedef uintptr_t BufferPointerType;

}
