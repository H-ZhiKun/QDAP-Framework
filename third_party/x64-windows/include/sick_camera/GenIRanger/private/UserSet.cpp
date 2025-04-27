// Copyright 2020 SICK AG. All rights reserved.

#include "../public/UserSet.h"

#include "../public/Exceptions.h"
#include "../public/GenIRanger.h"
#include "../public/NodeHelpers.h"
#include "../public/ScopedNodeValue.h"

namespace geniranger {

UserSetId userSetIdFromString(const std::string& genapiName)
{
  if (genapiName == "Default")
  {
    return UserSetId::DEFAULT;
  }
  else if (genapiName == "UserSet1")
  {
    return UserSetId::USER_SET_1;
  }
  else if (genapiName == "UserSet2")
  {
    return UserSetId::USER_SET_2;
  }
  else if (genapiName == "UserSet3")
  {
    return UserSetId::USER_SET_3;
  }
  else if (genapiName == "UserSet4")
  {
    return UserSetId::USER_SET_4;
  }
  else if (genapiName == "UserSet5")
  {
    return UserSetId::USER_SET_5;
  }
  throw std::invalid_argument(std::string("Unknown user set: ") + genapiName);
}

std::string toString(UserSetId userSet)
{
  switch (userSet)
  {
  case UserSetId::DEFAULT: return "Default";
  case UserSetId::USER_SET_1: return "UserSet1";
  case UserSetId::USER_SET_2: return "UserSet2";
  case UserSetId::USER_SET_3: return "UserSet3";
  case UserSetId::USER_SET_4: return "UserSet4";
  case UserSetId::USER_SET_5: return "UserSet5";
  }
  throw std::invalid_argument("Unknown user set id");
}

bool firmwareSupportsUserSets(NodeMap deviceNodeMap)
{
  return deviceNodeMap.isEnum("UserSetSelector");
}

UserSet::UserSet(NodeMap deviceNodeMap, UserSetId userSetId)
  : mDeviceNodeMap(deviceNodeMap)
  , mUserSetId(userSetId)
{
}

std::string UserSet::getDescription() const
{
  ScopedEnumValue userSetSelector(mDeviceNodeMap, "UserSetSelector", getName());
  return mDeviceNodeMap.getString("UserSetDescription");
}


void UserSet::setDescription(const std::string& description)
{
  ScopedEnumValue userSetSelector(mDeviceNodeMap, "UserSetSelector", getName());
  mDeviceNodeMap.setString("UserSetDescription", description);
}

bool UserSet::exists() const
{
#if 0
	if (mUserSetId == UserSetId::DEFAULT)
	{
		return true;
	}
	// If the GenICam file corresponding to the user set exists, we know the user
	// set also exists
	ScopedEnumValue fileSelector(mDeviceNodeMap, "FileSelector", getName());
	ScopedEnumValue fileOperationSelector(
		mDeviceNodeMap, "FileOperationSelector", "Open");
	ScopedEnumValue fileOpenMode(mDeviceNodeMap, "FileOpenMode", "Read");
	GenApi::EAccessMode executeCommandAccessMode =
		mDeviceNodeMap.getAccessMode("FileOperationExecute");
	return executeCommandAccessMode == GenApi::EAccessMode::RW; // VLB  
#endif // 0

  return true; // VLB
}

void UserSet::load()
{
  checkExistsAndMigrated("cannot be loaded");
  ScopedEnumValue userSetSelector(mDeviceNodeMap, "UserSetSelector", getName());
  mDeviceNodeMap.executeCommand("UserSetLoad");
}

void UserSet::save()
{
  if (mUserSetId == UserSetId::DEFAULT)
  {
    throw GenIRangerException("The default user set cannot be saved to");
  }
  ScopedEnumValue userSetSelector(mDeviceNodeMap, "UserSetSelector", getName());
  mDeviceNodeMap.executeCommand("UserSetSave");
}

std::string UserSet::retrieveFromCamera() const
{
  if (mUserSetId == UserSetId::DEFAULT)
  {
    throw GenIRangerException(
      "The default user set cannot be retrieved as a file");
  }
  
  // VLB Modified
  if (exists())
	  return geniranger::retrieveFileContents(getName(), mDeviceNodeMap.get()._Ptr);
  else
	  return "";
}

void UserSet::useAtStartup()
{
  checkExistsAndMigrated("cannot be used as startup user set");
  mDeviceNodeMap.setEnum("UserSetDefault", getName());
}

bool UserSet::needsMigration() const
{
  if (mUserSetId == UserSetId::DEFAULT || !exists())
  {
    return false;
  }

  // If the user set exists but cannot be loaded, it means it needs be migrated
  ScopedEnumValue userSetSelector(mDeviceNodeMap, "UserSetSelector", getName());
  GenApi::EAccessMode loadCommandAccessMode =
    mDeviceNodeMap.getAccessMode("UserSetLoad");
  return loadCommandAccessMode != GenApi::EAccessMode::RW;
}

ConfigurationResult UserSet::migrate()
{
  // Get current configuration to be able to leave device in same state when
  // done
  std::stringstream originalConfiguration;
  exportDeviceParameters(mDeviceNodeMap.get()._Ptr, originalConfiguration);
  originalConfiguration.seekg(0);

  // Download the user set as a GenICam file in CSV format
  std::string userSetConfiguration =
    retrieveFileContents(getName(), mDeviceNodeMap.get()._Ptr);

  // Import the user set
  std::istringstream userSetStream(userSetConfiguration);
  ConfigurationResult result =
    importDeviceParameters(mDeviceNodeMap.get()._Ptr, userSetStream);

  // Ensure the new configuration is valid
  if (!mDeviceNodeMap.getBool("DeviceRegistersValid"))
  {
    importDeviceParameters(mDeviceNodeMap.get()._Ptr, originalConfiguration);
    throw GenIRangerException(
      "The user set is not a valid configuration after migration, cannot save "
      "on device. You need to do manual migration!");
  }

  // Save the user set
  {
    ScopedEnumValue userSetSelector(
      mDeviceNodeMap, "UserSetSelector", getName());
    mDeviceNodeMap.executeCommand("UserSetSave");
  }

  // Re-import the original configuration
  importDeviceParameters(mDeviceNodeMap.get()._Ptr, originalConfiguration);

  return result;
}

void UserSet::checkExistsAndMigrated(const std::string& messageSuffix) const
{
  if (!exists())
  {
    std::ostringstream oss;
    oss << "The user set " << getName() << " doesn't exist on the device and "
        << messageSuffix;
    throw GenIRangerException(oss.str());
  }
  if (needsMigration())
  {
    std::ostringstream oss;
    oss << "The user set " << getName()
        << " needs migration to the current firmware and " << messageSuffix;
    throw GenIRangerException(oss.str());
  }
}

}
