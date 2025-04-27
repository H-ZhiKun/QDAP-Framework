// Copyright 2016-2020 SICK AG. All rights reserved.

#include "NodeTraverser.h"

#include "../public/Exceptions.h"
#include "GenIUtil.h"
#include "NodeUtil.h"

#include <algorithm>

using namespace GenApi;
using namespace GenICam;

// ----------------------------------------------------------------------------
// Helper functions

namespace {

/** Removes all nodes that are not "available" from the range. */
inline FeatureList_t::iterator
removeNotAvailableFeatures(FeatureList_t::iterator begin,
                           FeatureList_t::iterator end)
{
  return std::remove_if(
    begin, end, [](IValue* value) { return !IsAvailable(value->GetNode()); });
}

/** \return true if the list contains the node. */
inline bool containsNode(FeatureList_t& list, CNodePtr node)
{
  return std::find_if(
           list.begin(),
           list.end(),
           [&node](IValue* value) -> bool { return node == value->GetNode(); })
         != list.end();
}

/**
 * Get all the selectors for a node, including selectors in a hierarchy.
 *
 * \param node the selected node to find selectors for
 * \param selectors the found selectors
 */
void getSelectors(CNodePtr node, FeatureList_t& selectors)
{
  FeatureList_t nodeSelectors;
  static_cast<CSelectorPtr>(node)->GetSelectingFeatures(nodeSelectors);
  for (auto iter = nodeSelectors.begin(); iter != nodeSelectors.end(); ++iter)
  {
    if (!containsNode(selectors, *iter))
    {
      selectors.push_back(*iter);
    }
  }

  // In some cases not all selectors for a feature are found. We need
  // to traverse upwards to add missing selectors, thus a recursive
  // call.
  for (auto iter = nodeSelectors.begin(); iter != nodeSelectors.end(); ++iter)
  {
    getSelectors(CNodePtr(*iter), selectors);
  }
}

}

// ----------------------------------------------------------------------------
// NodeTraverser

namespace geniranger {

void NodeTraverser::traverse(CNodePtr node)
{
  CCategoryPtr category = static_cast<CCategoryPtr>(node);
  auto name = node->GetName();
  if (category.IsValid())
  {
    enterCategory(category);
    FeatureList_t features;
    category->GetFeatures(features);
    for (FeatureList_t::iterator it = features.begin(); it != features.end();
         it++)
    {
      traverse(*it);
    }
    leaveCategory(category);
  }
  else
  {
    // Apparently all writable nodes can be casted to a selector
    CSelectorPtr selector = static_cast<CSelectorPtr>(node);
    if (selector.IsValid())
    {
      FeatureList_t selectors;
      getSelectors(node, selectors);

      if (!selectors.empty())
      {
        // There are features selecting this node, iterate over all possible
        // values it can have.
        FeatureList_t::iterator begin = selectors.begin();
        FeatureList_t::iterator end = selectors.end();
        end = removeNotAvailableFeatures(begin, end);
        if (begin != end)
        {
          // Recursively go over all selector combinations for the node
          iterateAllSelectorValues(node, selectors);
        }
      }
      else if (selector->IsSelector())
      {
        // Selector, don't do anything. Its value shouldn't be stored.
      }
      else
      {
        // Feature node without dependencies to selectors
        onLeaf(node);
      }
    }
    else
    {
      // Skip nodes that aren't writable
    }
  }
}

void NodeTraverser::enterCategory(CCategoryPtr node)
{
  // Do nothing by default
}

void NodeTraverser::leaveCategory(CCategoryPtr node)
{
  // Do nothing by default
}

void NodeTraverser::enterSelector(CNodePtr selector)
{
  // Do nothing by default
}

void NodeTraverser::leaveSelector(CNodePtr node)
{
  // Do nothing by default
}

void NodeTraverser::onLeaf(CNodePtr node)
{
  // Do nothing by default
}

/**
 * Helper function to iterate the values of a single integer selector.
 * Recursively calls \ref iterateSelectorValues.
 */
void NodeTraverser::iterateIntSelectorValues(CNodePtr node,
                                             CNodePtr selector,
                                             const FeatureList_t& list)
{
  assert(nodeutil::isInteger(selector));
  CIntegerPtr intSelector = static_cast<CIntegerPtr>(selector);
  int64_t min = intSelector->GetMin();
  int64_t max = intSelector->GetMax();

  // Assuming continuous value range
  for (int64_t i = min; i <= max; i++)
  {
    try
    {
      intSelector->SetValue(i);
    }
    catch (GenericException e)
    {
      std::stringstream ss;
      ss << "Cannot set " << selector->GetName() << " to " << i
         << ". Library exception: " << e.GetDescription();
      geniutil::throwAndLog(ss.str());
    }

    if (!iterateAllSelectorValues(node, list, selector))
    {
      // All selectors for the node have been set so we can call
      // onLeaf() to perform the operation on the node.
      onLeaf(node);
    }
  }
}

/**
 * Helper function to iterate the values of a single enum selector. Recursively
 * calls \ref iterateSelectorValues.
 */
void NodeTraverser::iterateEnumSelectorValues(CNodePtr node,
                                              CNodePtr selector,
                                              const FeatureList_t& list)
{
  assert(nodeutil::isEnumeration(selector));

  CEnumerationPtr enumSelector = static_cast<CEnumerationPtr>(selector);
  NodeList_t entries;
  enumSelector->GetEntries(entries);

  for (NodeList_t::iterator it = entries.begin(); it != entries.end(); it++)
  {
    CEnumEntryPtr entry = static_cast<CEnumEntryPtr>(*it);

    // Skip all entries that are not available or not implemented, such as
    // unavailable regions for RegionSelector.
    if (!IsAvailable(entry->GetNode()))
    {
      continue;
    }

    try
    {
      enumSelector->SetIntValue(entry->GetValue());
    }
    catch (GenericException& e)
    {
      std::stringstream ss;
      ss << "Cannot set " << selector->GetName() << " to "
         << entry->GetSymbolic()
         << ". Library exception: " << e.GetDescription();
      // Leave selectors so that they aren't kept to next node
      geniutil::throwAndLog(ss.str());
    }

    if (!iterateAllSelectorValues(node, list, selector))
    {
      // All selectors for the node have been set so we can call
      // onLeaf() to perform the operation on the node.
      onLeaf(node);
    }
  }
}

/**
 * Iterate selectors from top level and down, i.e., we assume nested selectors
 * have a hierarchy of selecting each other.
 *
 * \param node the node to perform an action on for each combination of selector
 *        values
 * \param list the list of selectors for the node
 * \param parent the parent selector in this loop of recursion, or nullptr for
 *        entry to recursion
 * \return true if there were any selector with parent selecting it
 */
bool NodeTraverser::iterateAllSelectorValues(CNodePtr node,
                                             const FeatureList_t& list,
                                             CNodePtr parent)
{
  // Find the selectors that are selected by "parent", entry call
  // should be with nullptr parent to find the selector(s) with no
  // parents. Recursive calls are done downwards until a selector with
  // no child selectors is found, then we call the onLeaf() function.
  bool result = false;
  for (FeatureList_t::const_iterator iter = list.begin(); iter != list.end();
       ++iter)
  {
    CNodePtr selectorNode = *iter;
    CSelectorPtr selector = static_cast<CSelectorPtr>(selectorNode);
    assert(selector->IsSelector());

    // Get the selectors selecting this selector.
    FeatureList_t parentSelectors;
    selector->GetSelectingFeatures(parentSelectors);

    if (parentSelectors.empty() && parent == nullptr
        || containsNode(parentSelectors, parent))
    {
      result = true;
      enterSelector(selectorNode);
      // Automatically leave the selector when out of scope is reached, either
      // by returning normally or when an exception is thrown.
      auto deleter = [&](void*) { leaveSelector(selectorNode); };
      std::unique_ptr<void, decltype(deleter)> ptr(new int, deleter);

      if (nodeutil::isInteger(selectorNode))
      {
        iterateIntSelectorValues(node, selectorNode, list);
      }
      else if (nodeutil::isEnumeration(selectorNode))
      {
        iterateEnumSelectorValues(node, selectorNode, list);
      }
      else
      {
        std::stringstream ss;
        ss << "Unsupported selector: " << selectorNode->GetName() << std::endl;
        geniutil::throwAndLog(ss.str());
      }
    }
  }
  return result;
}

}
