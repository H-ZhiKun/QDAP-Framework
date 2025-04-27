// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include "Avoid.h"

#include <TLI/GenTL.h>

#include <memory>
#include <vector>

namespace gentlcpp {
class Buffer;
class DataStream;
}

namespace geniranger {

/**
 * Keeps tracks of allocated memory and GenTL buffer handles. The memory is
 * owned by this class and freed with \ref revokeAndFreeAllBuffers or by the
 * destructor.
 *
 * A buffer can be in one of the following states:
 * - Allocated and announced: memory and a buffer handle is allocated
 * - Queued in input pool: ready for being filled with received data
 * - Fill: currently being filled with received data
 * - Pending in output queue: filled with data and ready to be processed
 *
 * For more details, see the GenTL documentation chapter 5: Acquisition Engine.
 */
class BufferManager
{
public:
  explicit BufferManager(std::shared_ptr<gentlcpp::DataStream> dataStream)
    : mDataStream(dataStream)
  {
  }

  ~BufferManager();

  /**
   * Allocate memory for one buffer and announce it to the data stream.
   *
   * \return A handle to the newly allocated buffer
   */
  std::shared_ptr<gentlcpp::Buffer> allocateAndAnnounce(size_t byteCount);

  /**
   * Allocate memory for several buffers at once and announce them to the data
   * stream.
   */
  void allocateAndAnnounce(size_t bufferCount, size_t byteCount);

  /** Queues one buffer into the input pool. */
  void queueBuffer(std::shared_ptr<gentlcpp::Buffer> buffer);

  /** Queue all announced buffers into the input pool. */
  void queueAllBuffers();

  /**
   * Discard all buffers from the data stream. This will put them in the
   * announced state.
   */
  void discardAllBuffers();

  /** Revokes all buffers from the data stream and frees the memory. */
  void revokeAndFreeAllBuffers();

  /** \return a pointer to the start of the buffer data for a given handle. */
  uint8_t* dataPtrFromHandle(GenTL::BUFFER_HANDLE handle) const;

  std::shared_ptr<gentlcpp::Buffer>
  bufferFromHandle(GenTL::BUFFER_HANDLE handle) const;

  /** \return the allocated memory in bytes */
  size_t calculateAllocatedMemory() const;

  /**
   * \return the underlying \ref gentlcpp::DataStream
   * \note Normally this function should not be used. Solely added to allow
   *       work-arounds when functionality is missing
   */
  AVOID std::shared_ptr<gentlcpp::DataStream> getDataStream();

private:
  /** An allocated buffer with both memory and a handle */
  class BufferAllocation
  {
  public:
    std::shared_ptr<gentlcpp::Buffer> buffer;
    uint8_t* bufferData;
  };

private:
  const BufferAllocation& fromHandle(GenTL::BUFFER_HANDLE handle) const;

private:
  std::shared_ptr<gentlcpp::DataStream> mDataStream;
  std::vector<BufferAllocation> mAllocatedBuffers;
};

}
