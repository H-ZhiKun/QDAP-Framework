// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

#include "../public/GenIRangerDataTypes.h"

#include <istream>
#include <map>
#include <set>
#include <string>
#include <vector>

/** CSV reader that is closely connected to NodeExporter and its format. */
namespace geniranger {
typedef std::map<std::string, std::string> NodeValueMap;

class ConfigReader
{
public:
  ConfigReader();
  ~ConfigReader();

  ConfigurationResult parse(std::istream& csv);

  bool hasValue(std::string& key);
  std::string getValue(std::string& key);
  /** Returns all keys that exists in the configuration file. */
  std::set<std::string> getKeys();

private:
  // Node name (and selectors) is the key
  NodeValueMap mNodeValueMap;
};

}
