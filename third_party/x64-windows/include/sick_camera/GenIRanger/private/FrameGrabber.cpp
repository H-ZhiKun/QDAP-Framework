// Copyright 2019-2020 SICK AG. All rights reserved.

#include "../public/FrameGrabber.h"

#include "../public/frame/Frame.h"
#include "../public/gentlcpp/DataStream.h"
#include "../public/gentlcpp/NewBufferEvent.h"

namespace geniranger {

std::shared_ptr<FrameGrabber>
FrameGrabber::create(NodeMap deviceNodeMap,
                     std::shared_ptr<gentlcpp::DataStream> dataStream,
                     size_t bufferCount)
{
  return std::shared_ptr<FrameGrabber>(
    new FrameGrabber(deviceNodeMap, dataStream, bufferCount));
}

FrameGrabber::~FrameGrabber()
{
  if (isStarted())
  {
    stop();
  }
}

void FrameGrabber::start()
{
  // Lock parameters before starting
  mDeviceNodeMap.setInt("TLParamsLocked", 1);

  *mFrameInfo = geniranger::frame::fromDeviceConfiguration(mDeviceNodeMap);

  // Start receiver
  mEvent = mDataStream->registerNewBufferEvent();
  mDataStream->startAcquisition(GenTL::ACQ_START_FLAGS_DEFAULT, GENTL_INFINITE);

  // Start device
  mDeviceNodeMap.executeCommand("AcquisitionStart");
}

void FrameGrabber::stop()
{
  stopDevice();

  // Don't stop data stream in case startAcquisition() has failed
  if (mDataStream->isGrabbing())
  {
    mDataStream->stopAcquisition(GenTL::ACQ_STOP_FLAGS_KILL);
  }
  mEvent.reset();
}

void FrameGrabber::stopDevice()
{
  mDeviceNodeMap.executeCommand("AcquisitionStop");
  mDeviceNodeMap.setInt("TLParamsLocked", 0);
}

bool FrameGrabber::isStarted() const
{
  return mEvent != nullptr;
}

size_t FrameGrabber::getFrameBufferSize() const
{
  return mBuffers.calculateAllocatedMemory();
}

std::shared_ptr<geniranger::frame::IFrame>
FrameGrabber::grab(std::chrono::milliseconds timeout)
{
  std::shared_ptr<gentlcpp::Buffer> raw = grabRaw(timeout);
  return geniranger::frame::fromBuffer(
    raw, mDeviceNodeMap, *mFrameInfo, shared_from_this());
}

std::shared_ptr<geniranger::frame::IFrame> FrameGrabber::grab()
{
  std::shared_ptr<gentlcpp::Buffer> raw = grabRaw();
  return geniranger::frame::fromBuffer(
    raw, mDeviceNodeMap, *mFrameInfo, shared_from_this());
}

std::shared_ptr<gentlcpp::Buffer>
FrameGrabber::grabRaw(std::chrono::milliseconds timeout)
{
  GenTL::S_EVENT_NEW_BUFFER newBufferEvent = mEvent->getData(timeout);
  return mBuffers.bufferFromHandle(newBufferEvent.BufferHandle);
}

std::shared_ptr<gentlcpp::Buffer> FrameGrabber::grabRaw()
{
  GenTL::S_EVENT_NEW_BUFFER newBufferEvent = mEvent->getData();
  return mBuffers.bufferFromHandle(newBufferEvent.BufferHandle);
}

void FrameGrabber::queueBuffer(std::shared_ptr<gentlcpp::Buffer> buffer)
{
  mBuffers.queueBuffer(buffer);
}

NodeMap FrameGrabber::getDeviceNodeMap()
{
  return mDeviceNodeMap;
}

std::shared_ptr<gentlcpp::DataStream> FrameGrabber::getDataStream()
{
  return mDataStream;
}

BufferManager& FrameGrabber::getBufferManager()
{
  return mBuffers;
}

FrameGrabber::FrameGrabber(NodeMap deviceNodeMap,
                           std::shared_ptr<gentlcpp::DataStream> dataStream,
                           size_t bufferCount)
  : mDeviceNodeMap(deviceNodeMap)
  , mDataStream(dataStream)
  , mBuffers(mDataStream)
  , mFrameInfo(new frame::FrameConfigurationInfo())
{
  mBuffers.allocateAndAnnounce(bufferCount,
                               mDeviceNodeMap.getInt("PayloadSize"));
  mBuffers.queueAllBuffers();
}

}
