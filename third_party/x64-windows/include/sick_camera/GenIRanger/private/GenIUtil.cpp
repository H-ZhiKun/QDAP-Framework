// Copyright 2016-2020 SICK AG. All rights reserved.

#include "GenIUtil.h"

#include "../public/Exceptions.h"

#include <iostream>

namespace geniranger { namespace geniutil {

void log(std::string message)
{
#ifdef LOG_ENABLED
  std::cout << message;
#endif
}

void throwAndLog(std::string message)
{
#ifdef LOG_ENABLED
  std::cout << message << std::endl;
#endif
  throw GenIRangerException(message);
}

}}
