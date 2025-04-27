// Copyright 2019-2020 SICK AG. All rights reserved.
#pragma once

#include <GenApi/NodeMapRef.h>
#include "GenIRangerDll.h"// VLB Modified

#include <functional>
#include <memory>

namespace gentlcpp {
class Port;
}

namespace geniranger {

/**
 * Wrapper class for GenApi node maps. Makes sure to hold a reference to the
 * port that is needed for the node map to work.
 *
 * Contains convenience functions to get and set node values.
 */
class  NodeMap
{
public:
	GENIRANGER_API	NodeMap(std::shared_ptr<gentlcpp::Port> port);
	GENIRANGER_API  NodeMap(std::shared_ptr<gentlcpp::Port> port, const std::string & name); // VLB Modified
	GENIRANGER_API  NodeMap(const NodeMap&) = default;
	GENIRANGER_API  NodeMap& operator=(const NodeMap&) = default;

  /**
   * Operator to allow implicit conversion to the underlying GenApi node map.
   */
	operator GenApi::CNodeMapRef() { return mNodeMap; }
  /**
   * Operator to allow implicit conversion to the underlying GenApi node map.
   */
	operator const GenApi::CNodeMapRef() const { return mNodeMap; }

  /** Function to explicitly get the underlying GenApi node map. */
	GENIRANGER_API  GenApi::CNodeMapRef get() { return mNodeMap; }
  /** Function to explicitly get the underlying GenApi node map. */
	GENIRANGER_API  const GenApi::CNodeMapRef get() const { return mNodeMap; }

  /** \return the access mode of a node */
	GenApi::EAccessMode getAccessMode(const std::string& nodeName) const;

  /** \return true if the node is a command */
	bool isCommand(const std::string& nodeName) const;

  /** \return true if the node is a boolean */
	bool isBool(const std::string& nodeName) const;

  /** \return true if the node is a float */
	bool isFloat(const std::string& nodeName) const;

  /** \return true if the node is an integer */
	bool isInt(const std::string& nodeName) const;

  /** \return true if the node is an enumeration */
	bool isEnum(const std::string& nodeName) const;

  /** \return true if the node is a string */
	bool isString(const std::string& nodeName) const;

  /** Executes a command node */
	void executeCommand(const std::string& nodeName,
                      bool waitForCompletion = true);

  /** \return the value of a boolean node */
	bool getBool(const std::string& nodeName) const;

  /** \return the value of a float node */
	float getFloat(const std::string& nodeName) const;

  /** \return the value of an integer node */
  int64_t getInt(const std::string& nodeName) const;

  /** \return the value of an enumeration node as a string */
  std::string getEnum(const std::string& nodeName) const;
  /** \return the value of an enumeration node as an integer */
  int64_t getEnumInt(const std::string& nodeName) const;

  /** \return the value of a string node */
  std::string getString(const std::string& nodeName) const;

  /** Sets the value of a boolean node */
  void setBool(const std::string& nodeName, bool value, bool verify = true);

  /** Sets the value of a float node */
  void setFloat(const std::string& nodeName, float value, bool verify = true);

  /** Sets the value of an integer node */
  void setInt(const std::string& nodeName, int64_t value, bool verify = true);

  /** Sets the value of an enumeration node */
  void setEnum(const std::string& nodeName, const std::string& value);

  /** Sets the value of a string node */
  void setString(const std::string& nodeName,
                 const std::string& value,
                 bool verify = true);

  /** \return the min allowed value for a float node */
  float getFloatMin(const std::string& nodeName) const;
  /** \return the max allowed value for a float node */
  float getFloatMax(const std::string& nodeName) const;

  /** \return the min allowed value for an integer node */
  int64_t getIntMin(const std::string& nodeName) const;
  /** \return the max allowed value for an integer node */
  int64_t getIntMax(const std::string& nodeName) const;

//protected://Modified by VLB
public: //Modified by VLB
  // Allows implementation of sub-class that doesn't manage the combination of
  // port/node map
	GENIRANGER_API NodeMap(GenApi::CNodeMapRef nodeMap);
	GENIRANGER_API NodeMap() {};

protected:
  // Hold a reference to keep the port open
  std::shared_ptr<gentlcpp::Port> mPort;

  GenApi::CNodeMapRef mNodeMap;
};

}
