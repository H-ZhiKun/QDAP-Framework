// Copyright 2019-2020 SICK AG. All rights reserved.

#include "../public/BufferManager.h"

#include "../public/Exceptions.h"
#include "../public/gentlcpp/Buffer.h"
#include "../public/gentlcpp/DataStream.h"

#include <string>

namespace geniranger {

BufferManager::~BufferManager()
{
  discardAllBuffers();
  revokeAndFreeAllBuffers();
}

std::shared_ptr<gentlcpp::Buffer>
BufferManager::allocateAndAnnounce(size_t byteCount)
{
  BufferAllocation allocation;
  allocation.bufferData = new uint8_t[byteCount];
  allocation.buffer =
    mDataStream->announceBuffer(allocation.bufferData, byteCount, nullptr);
  mAllocatedBuffers.push_back(allocation);
  return allocation.buffer;
}

void BufferManager::allocateAndAnnounce(size_t bufferCount, size_t byteCount)
{
  for (size_t i = 0; i < bufferCount; ++i)
  {
    allocateAndAnnounce(byteCount);
  }
}

void BufferManager::queueBuffer(std::shared_ptr<gentlcpp::Buffer> buffer)
{
  mDataStream->queueBuffer(buffer);
}

void BufferManager::queueAllBuffers()
{
  for (auto& allocation : mAllocatedBuffers)
  {
    queueBuffer(allocation.buffer);
  }
}

void BufferManager::discardAllBuffers()
{
  mDataStream->flushQueue(GenTL::ACQ_QUEUE_ALL_DISCARD);
}

uint8_t* BufferManager::dataPtrFromHandle(GenTL::BUFFER_HANDLE handle) const
{
  return fromHandle(handle).bufferData;
}

std::shared_ptr<gentlcpp::Buffer>
BufferManager::bufferFromHandle(GenTL::BUFFER_HANDLE handle) const
{
  return fromHandle(handle).buffer;
}

void BufferManager::revokeAndFreeAllBuffers()
{
  for (auto& allocation : mAllocatedBuffers)
  {
    // Revoke buffer from stream so that memory can be deleted safely
    mDataStream->revokeBuffer(allocation.buffer);
    delete[] allocation.bufferData;
  }
  mAllocatedBuffers.clear();
}

const BufferManager::BufferAllocation&
BufferManager::fromHandle(GenTL::BUFFER_HANDLE handle) const
{
  for (auto& allocation : mAllocatedBuffers)
  {
    if (allocation.buffer->getHandle() == handle)
    {
      return allocation;
    }
  }
  std::ostringstream ss;
  ss << "Could not find buffer with handle: " << handle;
  throw GenIRangerException(ss.str());
}

size_t BufferManager::calculateAllocatedMemory() const
{
  size_t total = 0;
  for (auto& allocation : mAllocatedBuffers)
  {
    total += allocation.buffer->getSize();
  }
  return total;
}

std::shared_ptr<gentlcpp::DataStream> BufferManager::getDataStream()
{
  return mDataStream;
}

}
