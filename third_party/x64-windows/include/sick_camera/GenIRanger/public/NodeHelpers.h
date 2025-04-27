// Copyright 2019-2020 SICK AG. All rights reserved.

#pragma once

#include <GenApi/NodeMapRef.h>

#include <cstdint>

namespace geniranger {

/** Gets the access mode of a nodemap parameter */
extern GenApi::EAccessMode getAccessMode(GenApi::CNodeMapRef nodemap,
                                         const char* name);

/** Executes a nodemap command */
extern void executeCommand(GenApi::CNodeMapRef nodemap,
                           const char* name,
                           bool waitForCompletion = true);


// ----------------------------------------------------------------------------
// Functions to check type of a node

/** Checks if a nodemap parameter exists and is of the requested type */
template<class NodeType>
extern bool isType(GenApi::CNodeMapRef nodemap, const char* name);
const auto isBool = isType<GenApi::CBooleanPtr>;
const auto isCommand = isType<GenApi::CCommandPtr>;
const auto isEnum = isType<GenApi::CEnumerationPtr>;
const auto isFloat = isType<GenApi::CFloatPtr>;
const auto isInt = isType<GenApi::CIntegerPtr>;
const auto isString = isType<GenApi::CStringPtr>;


// ----------------------------------------------------------------------------
// Functions to get value

/** Gets a nodemap parameter value */
template<class NodeType, typename ValueType>
extern ValueType getNodeValue(GenApi::CNodeMapRef nodemap, const char* name);
const auto getEnum = getNodeValue<GenApi::CEnumerationPtr, GenICam::gcstring>;
const auto getInt = getNodeValue<GenApi::CIntegerPtr, int64_t>;
const auto getString = getNodeValue<GenApi::CStringPtr, GenICam::gcstring>;
/** Explicit template specialization for bool due to GenApi irregularity */
template<>
bool getNodeValue<GenApi::CBooleanPtr>(GenApi::CNodeMapRef nodemap,
                                       const char* name);
const auto getBool = getNodeValue<GenApi::CBooleanPtr, bool>;
/** Explicit template specialization for float due to type conversion */
template<>
float getNodeValue<GenApi::CFloatPtr>(GenApi::CNodeMapRef nodemap,
                                      const char* name);
const auto getFloat = getNodeValue<GenApi::CFloatPtr, float>;

/** Calls GetIntValue on an enumeration nodemap parameter */
extern int64_t getEnumInt(GenApi::CNodeMapRef nodemap, const char* name);

/**
 * Gets the value of a nodemap parameter as a string Differs from getString in
 * that this function can be called on many kinds of node types, while getString
 * makes sure the node is a string parameter.
 */
extern std::string toString(GenApi::CNodeMapRef nodemap, const char* name);


// ----------------------------------------------------------------------------
// Functions to get min/max values

/** Gets the maximum valid value of a nodemap parameter */
template<class NodeType, typename ValueType>
extern ValueType getNodeMax(GenApi::CNodeMapRef nodemap, const char* name);
const auto getIntMax = getNodeMax<GenApi::CIntegerPtr, int64_t>;
/** Explicit template specialization for float due to type conversion */
template<>
float getNodeMax<GenApi::CFloatPtr>(GenApi::CNodeMapRef nodemap,
                                    const char* name);
const auto getFloatMax = getNodeMax<GenApi::CFloatPtr, float>;

/** Gets the minimum valid value of a nodemap parameter */
template<class NodeType, typename ValueType>
extern ValueType getNodeMin(GenApi::CNodeMapRef nodemap, const char* name);
const auto getIntMin = getNodeMin<GenApi::CIntegerPtr, int64_t>;
/** Explicit template specialization for float due to type conversion */
template<>
float getNodeMin<GenApi::CFloatPtr>(GenApi::CNodeMapRef nodemap,
                                    const char* name);
const auto getFloatMin = getNodeMin<GenApi::CFloatPtr, float>;


// ----------------------------------------------------------------------------
// Functions to set value

/** Sets a nodemap parameter value with verification */
template<class NodeType, typename ValueType>
extern void
setNodeValue(GenApi::CNodeMapRef nodemap, const char* name, ValueType value);

/** Sets a nodemap parameter value with or without verification */
template<class NodeType, typename ValueType>
extern void setNodeValueExplicitVerification(GenApi::CNodeMapRef nodemap,
                                             const char* name,
                                             ValueType value,
                                             bool verify);

const auto setBool = setNodeValue<GenApi::CBooleanPtr, bool>;
const auto setEnum = setNodeValue<GenApi::CEnumerationPtr, const char*>;
const auto setFloat = setNodeValue<GenApi::CFloatPtr, float>;
const auto setInt = setNodeValue<GenApi::CIntegerPtr, int64_t>;
const auto setString = setNodeValue<GenApi::CStringPtr, const char*>;

const auto setBoolExplicitVerification =
  setNodeValueExplicitVerification<GenApi::CBooleanPtr, bool>;
const auto setFloatExplicitVerification =
  setNodeValueExplicitVerification<GenApi::CFloatPtr, float>;
const auto setIntExplicitVerification =
  setNodeValueExplicitVerification<GenApi::CIntegerPtr, int64_t>;
const auto setStringExplicitVerification =
  setNodeValueExplicitVerification<GenApi::CStringPtr, const char*>;

/** Calls SetIntValue on an enumeration nodemap parameter */
extern void setEnumInt(GenApi::CNodeMapRef nodemap,
                       const char* name,
                       int64_t value,
                       bool verify = true);

}
