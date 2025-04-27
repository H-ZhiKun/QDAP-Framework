// Copyright 2019-2020 SICK AG. All rights reserved.

#pragma once

#include "Avoid.h"
#include "BufferManager.h"
#include "NodeMap.h"

#include <chrono>
#include <memory>

namespace gentlcpp {
class DataStream;
class NewBufferEvent;
class Buffer;
}

namespace geniranger {

namespace frame {
class IFrame;
struct FrameConfigurationInfo;
}

/**
 * Allows grabbing frames from a device.
 *
 * This class takes care of memory allocation and starting of both the GenTL
 * acquisition engine and telling the device to start acquisition.
 */
class FrameGrabber : public std::enable_shared_from_this<FrameGrabber>
{
public:
  /**
   * Creates a FrameGrabber instance and allocates buffers that will be used to
   * store received frames.
   *
   * The size of the buffers will depend on the configured payload size, i.e.,
   * the settings of the device. If you reconfigure the device in a way that
   * affects the payload size you need to create a new FrameGrabber.
   *
   * The number of buffers you should choose depends on your application but is
   * a trade-off between the amount of memory used and the risk of losing data
   * because buffers are not readily available.
   *
   * \param deviceNodeMap node map to the remote device
   * \param dataStream the data stream from the device
   * \param bufferCount the number of buffers to allocate
   */
  static std::shared_ptr<FrameGrabber>
  create(NodeMap deviceNodeMap,
         std::shared_ptr<gentlcpp::DataStream> dataStream,
         size_t bufferCount);

  ~FrameGrabber();

  /** Starts acquisition of both device and receiver */
  void start();

  /** Stops acquisition of both the device and receiver */
  void stop();

  /**
   * Stops the device acquisition.
   *
   * This is useful if the device should stop acquiring frame, but the receiver
   * should continue processing everything in the queue.
   */
  void stopDevice();

  /** \return if acquisition has been started */
  bool isStarted() const;

  /** \return the number of bytes of allocated buffer memory */
  size_t getFrameBufferSize() const;

  /**
   * Grabs a frame
   *
   * \param timeout If a frame is not received within this period, an exception
   *        will be thrown.
   */
  std::shared_ptr<frame::IFrame> grab(std::chrono::milliseconds timeout);

  /**
   * Grabs a frame with infinite timeout, i.e., the function will not return
   * unless a frame is received.
   */
  std::shared_ptr<frame::IFrame> grab();

  /**
   * Grab a frame and return the raw GenTL buffer object. Prefer using \ref grab
   * where possible.
   *
   * \param timeout If a buffer is not received within this period, an exception
   *        will be thrown.
   */
  std::shared_ptr<gentlcpp::Buffer> grabRaw(std::chrono::milliseconds timeout);

  /**
   * Grab a frame and return the raw GenTL buffer object, with infinite timeout,
   * i.e., the function will not return unless a buffer is received. Prefer
   * using \ref grab where possible.
   */
  std::shared_ptr<gentlcpp::Buffer> grabRaw();

  /**
   * Gives the buffer back to the acquisition engine, so that the memory may be
   * reused for subsequent frames.
   */
  void queueBuffer(std::shared_ptr<gentlcpp::Buffer> buffer);

  /**
   * \return the underlying \ref NodeMap for the device
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID NodeMap getDeviceNodeMap();

  /**
   * \return the underlying \ref gentlcpp::DataStream
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID std::shared_ptr<gentlcpp::DataStream> getDataStream();

  /**
   * \return the underlying \ref BufferManager for the device
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID BufferManager& getBufferManager();

private:
  FrameGrabber(NodeMap deviceNodeMap,
               std::shared_ptr<gentlcpp::DataStream> dataStream,
               size_t bufferCount);


private:
  NodeMap mDeviceNodeMap;
  std::shared_ptr<gentlcpp::DataStream> mDataStream;
  BufferManager mBuffers;
  std::shared_ptr<gentlcpp::NewBufferEvent> mEvent;
  std::unique_ptr<frame::FrameConfigurationInfo> mFrameInfo;
};

}
