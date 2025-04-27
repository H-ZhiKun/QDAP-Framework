// Copyright 2017-2020 SICK AG. All rights reserved.

#pragma once

#include "Base/GCNamespace.h"
#include "NodeTraverser.h"
#include <vector>

namespace geniranger {

/**
 * Saves and restores the state of all selector nodes in a NodeMap.
 *
 * The value of a selector should not affect the behavior of the device.
 * Nevertheless, it is nice if the state of the selectors are restored to their
 * previous values when exporting/importing a parameter file.
 *
 * When an instance is created, the node map is traversed and the value of every
 * selector is saved. When the instance is destroyed, the value of every
 * selector is restored.
 */
class SelectorSnapshot : private NodeTraverser
{
public:
  SelectorSnapshot(GenApi::INodeMap* nodeMap);
  ~SelectorSnapshot();

protected:
  virtual void enterSelector(GenApi::CNodePtr selector) override;
  virtual void leaveSelector(GenApi::CNodePtr selector) override;

private:
  struct SelectorState
  {
    GenICam::gcstring name;
    int64_t value;
  };

  GenApi::INodeMap* mNodeMap;
  std::vector<SelectorState> mState;
};

}
