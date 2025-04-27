// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

#include "GenICam.h"

namespace geniranger {
/** Base class for traversing GenICam node map structure */
class NodeTraverser
{
public:
  virtual ~NodeTraverser(){};

  /** Traverse the node tree starting from the specified node */
  virtual void traverse(GenApi::CNodePtr node);

protected:
  /** Performs an operation when entering the scope a category */
  virtual void enterCategory(GenApi::CCategoryPtr category);

  /** Performs an operation when leaving the scope of a category */
  virtual void leaveCategory(GenApi::CCategoryPtr category);

  /**
   * Performs an operation when encountering a selector. These aren't treated as
   * scopes like a category.
   */
  virtual void enterSelector(GenApi::CNodePtr selector);

  /**
   * Action to perform when the SelectorState returned from enterSelector goes
   * out of scope.
   */
  virtual void leaveSelector(GenApi::CNodePtr selector);


  /**
   * Performs an operation when encountering a node which isn't a category or
   * selector.
   */
  virtual void onLeaf(GenApi::CNodePtr node);

private:
  void iterateIntSelectorValues(GenApi::CNodePtr node,
                                GenApi::CNodePtr selector,
                                const GenApi::FeatureList_t& list);
  void iterateEnumSelectorValues(GenApi::CNodePtr node,
                                 GenApi::CNodePtr selector,
                                 const GenApi::FeatureList_t& list);
  bool iterateAllSelectorValues(GenApi::CNodePtr node,
                                const GenApi::FeatureList_t& list,
                                GenApi::CNodePtr parent = nullptr);
};

}
