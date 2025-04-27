// Copyright 2019-2020 SICK AG. All rights reserved.

#include "EnumSelectorEntries.h"
#include <algorithm>
#include <cassert>

namespace geniranger {

EnumSelectorEntries::EnumSelectorEntries(GenApi::CNodeMapRef device,
                                         const std::string& selectorName)
  : mSelectorName(selectorName)
{
  GenApi::CEnumerationPtr selector = device._GetNode(selectorName.c_str());
  GenApi::NodeList_t entries;
  selector->GetEntries(entries);
  for (GenApi::CEnumEntryPtr entry : entries)
  {
    std::string enumName{entry->GetSymbolic().c_str()};
    mValueFromName[enumName] = entry->GetValue();
  }
}

uint64_t EnumSelectorEntries::value(const std::string& name) const
{
  if (mValueFromName.find(name) == mValueFromName.end())
  {
    std::ostringstream ss;
    ss << "Cannot find entry with name " << name << " in " << mSelectorName;
    throw std::invalid_argument(ss.str());
  }
  return mValueFromName.at(name);
}

const std::string& EnumSelectorEntries::name(uint64_t value) const
{
  auto iter = std::find_if(
    mValueFromName.begin(),
    mValueFromName.end(),
    [=](std::pair<std::string, uint64_t> p) { return p.second == value; });
  if (iter == mValueFromName.end())
  {
    std::ostringstream ss;
    ss << "Cannot find name for value " << value << " in " << mSelectorName;
    throw std::invalid_argument(ss.str());
  }
  return iter->first;
}

}
