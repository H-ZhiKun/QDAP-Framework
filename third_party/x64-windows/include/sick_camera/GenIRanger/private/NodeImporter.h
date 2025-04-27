// Copyright 2016-2020 SICK AG. All rights reserved.

#pragma once

#include "ConfigReader.h"
#include "ConfigWriter.h"
#include "NodeTraverser.h"

#include <set>

namespace geniranger {

/** Opposite from NodeExporter, sets the value of each writable node. */
class NodeImporter : public NodeTraverser
{
public:
  NodeImporter(ConfigReader reader);
  ~NodeImporter();

  const std::vector<std::string>& getErrors() const;
  const std::vector<std::string>& getMissing() const;
  const std::vector<std::string>& getUnknown() const;

protected:
  /**
   * Saves the visited selector so that the key for the upcoming node can be
   * reconstructed.
   */
  virtual void enterSelector(GenApi::CNodePtr selector) override;
  /** Pops the latest visited selected */
  virtual void leaveSelector(GenApi::CNodePtr selector) override;

  virtual void leaveCategory(GenApi::CCategoryPtr category) override;

  /** Reads the imported value and sets it */
  virtual void onLeaf(GenApi::CNodePtr node) override;

  /**
   * Checks if there were any parameters in the configuration file that has not
   * been imported. This could happen if the parameter doesn't exist in the
   * device node map.
   */
  void checkUnimportedParameters();

  std::string makeKey(const std::string& nodeName);

private:
  ConfigReader mReader;
  std::vector<GenApi::CNodePtr> mSelectors;

  std::set<std::string> mRemainingToImport;
  std::vector<std::string> mErrors;
  std::vector<std::string> mMissing;
  std::vector<std::string> mUnknown;
};

}
