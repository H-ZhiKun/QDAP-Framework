// Copyright 2016-2020 SICK AG. All rights reserved.

#include "../../public/gentlcpp/Tracing.h"

#include <cstdio>
#include <stdarg.h>
#include <string>

namespace {

bool gTracingEnabled = false;

}

namespace gentlcpp {

void enableTracing(bool enabled)
{
  gTracingEnabled = enabled;
}

void trace(const char* fmt, ...)
{
  if (gTracingEnabled)
  {
    char buff[512];
    va_list args;
    va_start(args, fmt);
    vsprintf_s(buff, fmt, args);
    va_end(args);
    std::string result(buff);
    result += "\n";
    std::fprintf(stderr, "%s", result.c_str());
  }
}

}
