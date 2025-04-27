// Copyright 2020 SICK AG. All rights reserved.

#include "NodeMap.h"

namespace geniranger {

/**
 * Class that can be used to temporarily set a value of a node in a NodeMap.
 * Mainly used for selectors but can be used for any parameter. The class uses
 * RAII to ensure that the value is reset to its original value when the object
 * goes out of scope.
 *
 * Use \ref ScopedEnumValue and \ref ScopedIntValue depending on the type of the
 * selector.
 */
template<typename T>
class ScopedNodeValue
{
public:
  ScopedNodeValue(NodeMap nodeMap, const std::string& nodeName, T value);
  ~ScopedNodeValue();

private:
  T getValue();
  void setValue(T value);

private:
  NodeMap mNodeMap;
  std::string mNodeName;
  T mOriginalValue;
};

/** ScopedNodeValue for enumeration nodes. */
typedef ScopedNodeValue<std::string> ScopedEnumValue;

/** ScopedNodeValue for integer nodes. */
typedef ScopedNodeValue<int64_t> ScopedIntValue;


template<typename T>
ScopedNodeValue<T>::ScopedNodeValue(NodeMap nodeMap,
                                    const std::string& nodeName,
                                    T value)
  : mNodeMap(nodeMap)
  , mNodeName(nodeName)
{
  mOriginalValue = getValue();
  setValue(value);
}

template<typename T>
ScopedNodeValue<T>::~ScopedNodeValue()
{
  try
  {
    setValue(mOriginalValue);
  }
  catch (...)
  {
    // If we fail to reset the node it is likely because an exception has
    // been thrown in the scope of the RAII object. We are more likely
    // interested in throwing that exception so we ignore this.
  }
}

template<>
std::string ScopedNodeValue<std::string>::getValue()
{
  return mNodeMap.getEnum(mNodeName);
}

template<>
void ScopedNodeValue<std::string>::setValue(std::string value)
{
  mNodeMap.setEnum(mNodeName, value);
}

template<>
int64_t ScopedNodeValue<int64_t>::getValue()
{
  return mNodeMap.getInt(mNodeName);
}

template<>
void ScopedNodeValue<int64_t>::setValue(int64_t value)
{
  mNodeMap.setInt(mNodeName, value);
}

}
