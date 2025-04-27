// Copyright 2016-2020 SICK AG. All rights reserved.

#include "NodeImporter.h"

#include "../public/Exceptions.h"
#include "GenIUtil.h"
#include "NodeUtil.h"

#include <algorithm>
#include <sstream>

using namespace GenApi;
using namespace GenICam;

namespace geniranger {

NodeImporter::NodeImporter(ConfigReader reader)
  : mReader(reader)
  , mRemainingToImport(reader.getKeys())
{
}

NodeImporter::~NodeImporter()
{
}

void NodeImporter::enterSelector(CNodePtr selector)
{
  mSelectors.push_back(selector);
}

void NodeImporter::leaveSelector(CNodePtr node)
{
  mSelectors.pop_back();
}


void NodeImporter::leaveCategory(CCategoryPtr category)
{
  // When all nodes have been traversed
  auto categoryName = category->GetNode()->GetName(true);
  if (categoryName == "Std::Root")
  {
    checkUnimportedParameters();
  }
}

static bool compare(CNodePtr lh, CNodePtr rh)
{
  return lh->GetName() < rh->GetName();
}

void NodeImporter::onLeaf(CNodePtr node)
{
  std::string key(makeKey(std::string(node->GetName())));
  bool hasAlias = false;
  std::string aliasKey;
  if (node->GetAlias() != nullptr)
  {
    std::string alias = node->GetAlias()->GetName().c_str();
    aliasKey = makeKey(alias);
    hasAlias = true;
  }

  // Mark this key as imported even before importing to avoid getting an
  // error that something is not a configuration parameter even though it
  // is but has a faulty value.
  mRemainingToImport.erase(key);
  if (hasAlias)
  {
    mRemainingToImport.erase(aliasKey);
  }

  if (nodeutil::isConfigNode(node))
  {
    bool hasValue = false;
    std::string readValue;
    if (mReader.hasValue(key))
    {
      readValue = mReader.getValue(key);
      hasValue = true;
    }
    else if (hasAlias && mReader.hasValue(aliasKey))
    {
      readValue = mReader.getValue(aliasKey);
      hasValue = true;
    }

    if (hasValue)
    {
      try
      {
        auto type = node->GetPrincipalInterfaceType();
        if (type == intfIInteger)
        {
          CIntegerPtr value = static_cast<CIntegerPtr>(node);
          value->FromString(gcstring(readValue.c_str()), false);
        }
        else if (type == intfIFloat)
        {
          CFloatPtr value = static_cast<CFloatPtr>(node);
          value->FromString(gcstring(readValue.c_str()), false);
        }
        else if (type == intfIString)
        {
          CStringPtr value = static_cast<CStringPtr>(node);
          value->SetValue(gcstring(readValue.c_str()), false);
        }
        else if (type == intfIBoolean)
        {
          CBooleanPtr value = static_cast<CBooleanPtr>(node);
          value->FromString(gcstring(readValue.c_str()), false);
        }
        else if (type == intfIEnumeration)
        {
          CEnumerationPtr value = static_cast<CEnumerationPtr>(node);
          value->FromString(gcstring(readValue.c_str()), false);
        }
        else
        {
          // Do nothing, the other types aren't interesting
        }

        geniutil::log("Set " + key + " " + readValue + "\n");
      }
      catch (GenericException& e)
      {
        std::stringstream ss;
        ss << "Cannot set " << key << " = " << readValue
           << ". Library exception: " << e.GetDescription();

        mErrors.push_back(ss.str());
      }
      catch (std::exception& e)
      {
        std::ostringstream ss;
        ss << "Cannot set " << key << " = " << readValue << ". " << e.what();
        mErrors.push_back(ss.str());
      }
    }
    else
    {
      // The parameter is not available in the configuration
      mMissing.push_back(key);
    }
  }
  else
  {
    // Not a configuration node
    if (mReader.hasValue(key))
    {
      // Non-configuration parameters should not be in the configuration
      mUnknown.push_back(key);
    }
  }
}

void NodeImporter::checkUnimportedParameters()
{
  for (auto it = mRemainingToImport.begin(); it != mRemainingToImport.end();
       ++it)
  {
    mUnknown.push_back(*it);
  }
}

std::string NodeImporter::makeKey(const std::string& nodeName)
{
  std::string key = nodeName;
  if (mSelectors.size() > 0)
  {
    std::ostringstream ss;
    // Copy selector vector, so it can be sorted without modifying the original
    auto sorted = mSelectors;
    std::sort(sorted.begin(), sorted.end(), compare);
    for (auto it = sorted.begin(); it != sorted.end(); it++)
    {
      auto selector = *it;
      ss << "_" << selector->GetName();
      ss << "_" << nodeutil::getValueAsString(selector);
    }
    key.append(ss.str());
  }
  return key;
}

const std::vector<std::string>& NodeImporter::getErrors() const
{
  return mErrors;
}

const std::vector<std::string>& NodeImporter::getMissing() const
{
  return mMissing;
}

const std::vector<std::string>& NodeImporter::getUnknown() const
{
  return mUnknown;
}

}
