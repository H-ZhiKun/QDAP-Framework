// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "Exception.h"

#include <memory>

namespace gentlcpp {
class Event;

/**
 * Interface for all modules.
 *
 * Used in event handling, all created Events need to be able to check if the
 * module they're registered to is still open when accessed.
 */
class Module
{
public:
  virtual ~Module() noexcept {}
  virtual std::shared_ptr<Event>
  registerEvent(GenTL::EVENT_TYPE_LIST eventType) = 0;

  void ThrowIfError(GenTL::GC_ERROR status) const
  {
    gentlcpp::ThrowIfError(mCApi, status);
  }

protected:
  Module(std::shared_ptr<const CApi> cApi)
    : mCApi(cApi)
  {
  }

protected:
  std::shared_ptr<const CApi> mCApi;
};

}
