// Copyright 2019-2020 SICK AG. All rights reserved.
#include "../../public/frame/Component.h"
#include "../../public/Exceptions.h"
#include <iostream>

namespace geniranger { namespace frame {

ComponentId Component::getId() const
{
  return mComponentId;
}

geniranger::PixelFormat Component::getPixelFormat() const
{
  return mPixelFormat;
}

uint16_t Component::getWidth() const
{
  return mWidth;
}

uint16_t Component::getHeight() const
{
  return mHeight;
}

uint8_t* Component::getData() const
{
  return mData;
}

size_t Component::getDataSize() const
{
  return mDataSize;
}

std::shared_ptr<gentlcpp::BufferPart> Component::getBufferPart() const
{
  if (mBufferPart == nullptr)
  {
    throw NotAvailableException(
      "The Component was not created from a multi-part GenTL buffer");
  }
  return mBufferPart;
}

std::ostream& operator<<(std::ostream& s, const Component& component)
{
  s << "Component ID: " << toString(component.getId()) << std::endl;
  s << "Component Pixel Format: " << toString(component.getPixelFormat())
    << std::endl;
  s << "Component Height: " << component.getHeight() << std::endl;
  return s;
}

}}
