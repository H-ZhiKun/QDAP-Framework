// Copyright 2018-2020 SICK AG. All rights reserved.

#include "../public/ChunkAdapter.h"
#include "../public/Exceptions.h"
#include "../public/gentlcpp/Buffer.h"

namespace geniranger {

ChunkAdapter::ChunkAdapter(NodeMap nodeMap)
  : mNodeMap(nodeMap)
  , mAdapter(new GenApi::CChunkAdapterGEV())
{
  mAdapter->AttachNodeMap(nodeMap.get()._Ptr);
}

ChunkAdapter::ChunkAdapter(NodeMap nodeMap,
                           std::shared_ptr<gentlcpp::Buffer> buffer)
  : ChunkAdapter(nodeMap)
{
  attachBuffer(buffer);
}

ChunkAdapter::~ChunkAdapter()
{
  if (mBuffer != nullptr)
  {
    detachBuffer();
  }
  mAdapter->DetachNodeMap();
}

void ChunkAdapter::attachBuffer(std::shared_ptr<gentlcpp::Buffer> buffer)
{
  GenApi::AttachStatistics_t statistics;
  size_t chunkPayloadSize = buffer->getDeliveredChunkPayloadSize();
  uint8_t* bufferData = reinterpret_cast<uint8_t*>(buffer->getBase());

  if (!mAdapter->CheckBufferLayout(bufferData, chunkPayloadSize))
  {
    throw ChunkFormatException("Buffer has unknown chunk layout");
  }
  mAdapter->AttachBuffer(bufferData, chunkPayloadSize, &statistics);
  mBuffer = buffer;

  // Ranger3 uses a single chunk port for all metadata.
  if (statistics.NumChunkPorts != 1)
  {
    throw ChunkFormatException("A single chunk port was expected");
  }

  // There should be one chunk for the metadata port and one for
  // wrapping the image data.
  if (statistics.NumChunks != 2)
  {
    throw ChunkFormatException("Two chunks were expected");
  }

  // Only the metadata chunk should be attached.
  if (statistics.NumAttachedChunks != 1)
  {
    throw std::exception("A single attached chunk was expected");
  }
}

void ChunkAdapter::detachBuffer()
{
  mAdapter->DetachBuffer();
  mBuffer = nullptr;
}

}
