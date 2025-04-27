// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include "../../public/Avoid.h"
#include "BasicTypes.h"
#include "../../public/PixelFormat.h"

#include <memory>
#include <vector>
#include <stdexcept>

namespace gentlcpp {
class BufferPart;
}

namespace geniranger { namespace frame {

/**
 * Abstraction of a component in a \ref Region within a \ref IFrame. The
 * component can be used to access data within a GenTL buffer part.
 */
class Component
{
public:
  /** \return the \ref ComponentId of the component */
  ComponentId getId() const;

  /** \return the pixel format of the component */
  PixelFormat getPixelFormat() const;

  /** \return the width of the component in pixels on the sensor */
  uint16_t getWidth() const;
  /**
   * \return the height of the component. For a 3D extraction region, this is
   *         the number of lines scanned.
   */
  uint16_t getHeight() const;

  /**
   * \return a pointer to the data in the GenTL buffer part corresponding to the
   *         component
   */
  uint8_t* getData() const;
  /** \return the size of the data pointed to by \ref getData */
  size_t getDataSize() const;

  /**
   * \return the underlying \ref gentlcpp::BufferPart if the frame was created
   *         from a buffer and if the buffer is multi-part
   * \throw NotAvailableException if the frame was created some other way
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID std::shared_ptr<gentlcpp::BufferPart> getBufferPart() const;

private:
  friend class Region;
  friend class GenTlBackedFrame;

  Component(ComponentId componentId,
            PixelFormat pixelFormat,
            uint16_t width,
            uint16_t height,
            uint8_t* data,
            size_t dataSize)
    : mComponentId(componentId)
    , mPixelFormat(pixelFormat)
    , mWidth(width)
    , mHeight(height)
    , mData(data)
    , mDataSize(dataSize)
  {
  }

private:
  ComponentId mComponentId;
  PixelFormat mPixelFormat;
  uint16_t mWidth;
  uint16_t mHeight;
  uint16_t mOffsetX;
  uint16_t mOffsetY;
  uint8_t* mData;
  size_t mDataSize;

  std::shared_ptr<gentlcpp::BufferPart> mBufferPart;
};

typedef std::vector<std::shared_ptr<Component>> ComponentList;
typedef std::vector<std::shared_ptr<const Component>> ConstComponentList;

/** Logs information about a \ref Component. */
std::ostream& operator<<(std::ostream& s, const Component& component);

}}
