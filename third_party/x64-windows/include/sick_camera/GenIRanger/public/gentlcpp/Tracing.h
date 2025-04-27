// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

namespace gentlcpp {

/**
 * When enabled, creation/destruction of the C++ wrapper objects will be written
 * to stderr. This is useful for debugging and unit testing issues related to
 * garbage collection.
 */
void enableTracing(bool enable);

/** Writes to stderr, if tracing is enabled. */
void trace(const char* msg, ...);

}
