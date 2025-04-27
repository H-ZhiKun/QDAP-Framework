// Copyright 2016-2020 SICK AG. All rights reserved.
#pragma once

#include "Event.h"

#include <memory>

namespace gentlcpp {
class Module;

/** This is a convenience class for the "new buffer event" type */
class NewBufferEvent : public Event
{
public:
  NewBufferEvent(std::shared_ptr<const CApi> cApi,
                 std::shared_ptr<const Module> module,
                 GenTL::EVENT_TYPE_LIST eventType,
                 GenTL::EVENTSRC_HANDLE eventSrcHandle);
  ~NewBufferEvent() noexcept;

  /**
   * Wait for a new buffer to be available.
   *
   * \param timeout If a buffer is not received within this period, an exception
   *        will be thrown.
   */
  GenTL::S_EVENT_NEW_BUFFER getData(std::chrono::milliseconds timeout) const;

  /**
   * Wait for a new buffer to be available with infinite timeout, i.e., the
   * function will not return unless a buffer is received.
   */
  GenTL::S_EVENT_NEW_BUFFER getData() const;
};

}
