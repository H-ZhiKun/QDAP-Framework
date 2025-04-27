// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include "NodeMap.h"

#include <GenApi/ChunkAdapterGEV.h>
#include <memory>

namespace gentlcpp {
class Buffer;
}

namespace geniranger {

/**
 * Wrapper class for a GigE Vision chunk adapter. Creating an instance of this
 * class and attaching it to a node map and a buffer will make sure you can
 * access chunk meta data via the node map.
 */
class ChunkAdapter
{
public:
  explicit ChunkAdapter(NodeMap nodeMap);

  /**
   * Construct an adapter and immediately attach a buffer.
   *
   * \throw ChunkFormatException if the chunk format is incorrect.
   */
  ChunkAdapter(NodeMap nodeMap, std::shared_ptr<gentlcpp::Buffer> buffer);

  /** Detaches any attached buffer. */
  ~ChunkAdapter();

  /**
   * Attach the chunk adapter to a specific buffer. This allows access to the
   * chunk metadata carried in the buffer.
   *
   * \throw ChunkFormatException if the chunk format is incorrect.
   */
  void attachBuffer(std::shared_ptr<gentlcpp::Buffer> buffer);

  /** Detach the from the buffer when done with it. */
  void detachBuffer();

private:
  NodeMap mNodeMap;
  std::shared_ptr<gentlcpp::Buffer> mBuffer;
  std::unique_ptr<GenApi::CChunkAdapterGEV> mAdapter;
};

}
