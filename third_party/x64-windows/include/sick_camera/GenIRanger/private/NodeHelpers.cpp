// Copyright 2019-2020 SICK AG. All rights reserved.

#include "../public/NodeHelpers.h"
#include "NodeUtil.h"

#include <GenApi/IBoolean.h>
#include <GenApi/ICommand.h>
#include <GenApi/IString.h>

#include <chrono>
#include <thread>

namespace geniranger {

GenApi::EAccessMode getAccessMode(GenApi::CNodeMapRef nodemap, const char* name)
{
  GenApi::CBasePtr basePointer = nodemap._GetNode(name);
  return basePointer->GetAccessMode();
}

void executeCommand(GenApi::CNodeMapRef nodemap,
                    const char* name,
                    bool waitForCompletion)
{
  GenApi::CCommandPtr node = nodemap._GetNode(name);
  node->Execute();
  if (waitForCompletion)
  {
    while (!node->IsDone())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
}

template<class NodeType>
bool isType(GenApi::CNodeMapRef nodemap, const char* name)
{
  NodeType node = nodemap._GetNode(name);
  return node.IsValid();
}

template<class NodeType, typename ValueType>
ValueType getNodeValue(GenApi::CNodeMapRef nodemap, const char* name)
{
  NodeType node = nodemap._GetNode(name);
  return **node;
}

template<>
bool getNodeValue<GenApi::CBooleanPtr>(GenApi::CNodeMapRef nodemap,
                                       const char* name)
{
  GenApi::CBooleanPtr node = nodemap._GetNode(name);
  // GenApi::IBoolean does not have operator*() for some reason, so use
  // GetValue instead
  return node->GetValue();
}

template<>
float getNodeValue<GenApi::CFloatPtr>(GenApi::CNodeMapRef nodemap,
                                      const char* name)
{
  GenApi::CFloatPtr node = nodemap._GetNode(name);
  // GenApi::IFloat represents a double and not a float for some reason, so type
  // convert it. GigEVision floats are single precision so no actual precision
  // is lost
  return static_cast<float>(**node);
}

int64_t getEnumInt(GenApi::CNodeMapRef nodemap, const char* name)
{
  GenApi::CEnumerationPtr enumPointer = nodemap._GetNode(name);
  return enumPointer->GetIntValue();
}

std::string toString(GenApi::CNodeMapRef nodemap, const char* name)
{
  return nodeutil::getValueAsString(nodemap._GetNode(name));
}

template<class NodeType, typename ValueType>
ValueType getNodeMax(GenApi::CNodeMapRef nodemap, const char* name)
{
  NodeType node = nodemap._GetNode(name);
  return node->GetMax();
}

template<>
float getNodeMax<GenApi::CFloatPtr>(GenApi::CNodeMapRef nodemap,
                                    const char* name)
{
  GenApi::CFloatPtr node = nodemap._GetNode(name);
  return static_cast<float>(node->GetMax());
}

template<class NodeType, typename ValueType>
ValueType getNodeMin(GenApi::CNodeMapRef nodemap, const char* name)
{
  NodeType node = nodemap._GetNode(name);
  return node->GetMin();
}

template<>
float getNodeMin<GenApi::CFloatPtr>(GenApi::CNodeMapRef nodemap,
                                    const char* name)
{
  GenApi::CFloatPtr node = nodemap._GetNode(name);
  return static_cast<float>(node->GetMin());
}

template<class NodeType, typename ValueType>
void setNodeValue(GenApi::CNodeMapRef nodemap,
                  const char* name,
                  ValueType value)
{
  NodeType node = nodemap._GetNode(name);
  *node = value;
}

template<class NodeType, typename ValueType>
void setNodeValueExplicitVerification(GenApi::CNodeMapRef nodemap,
                                      const char* name,
                                      ValueType value,
                                      bool verify)
{
  NodeType node = nodemap._GetNode(name);
  node->SetValue(value, verify);
}

void setEnumInt(GenApi::CNodeMapRef nodemap,
                const char* name,
                int64_t value,
                bool verify)
{
  GenApi::CEnumerationPtr enumPointer = nodemap._GetNode(name);
  enumPointer->SetIntValue(value, verify);
}

}
