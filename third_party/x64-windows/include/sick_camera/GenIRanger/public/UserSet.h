// Copyright 2020 SICK AG. All rights reserved.
#pragma once

#include "GenIRangerDataTypes.h"
#include "NodeMap.h"

namespace geniranger {

/**
 * Identifier for different GenICam User Sets. Values are the same as for the
 * UserSetSelector to avoid confusion but you should not rely on this in your
 * code.
 */


enum class UserSetId
{
  DEFAULT = 0,
  USER_SET_1 = 1,
  USER_SET_2 = 2,
  USER_SET_3 = 3,
  USER_SET_4 = 4,
  USER_SET_5 = 5,
};

/** All user set idenitifiers in an iterable array. */
static const UserSetId allUserSetIds[] = {UserSetId::DEFAULT,
                                          UserSetId::USER_SET_1,
                                          UserSetId::USER_SET_2,
                                          UserSetId::USER_SET_2,
                                          UserSetId::USER_SET_3,
                                          UserSetId::USER_SET_4,
                                          UserSetId::USER_SET_5};

UserSetId userSetIdFromString(const std::string& genapiName);
std::string toString(UserSetId userSet);

/**
 * Earlier device firmwares do not support GenICam User Sets. Before trying to
 * access them on a device with unknown firmware it is a good idea to call this
 * function to check.
 */
bool firmwareSupportsUserSets(NodeMap deviceNodeMap);

/**
 * Representation of a GenICam User Set, i.e., a configuration stored on the
 * device.
 */
class UserSet
{
public:
  explicit GENIRANGER_API UserSet(NodeMap deviceNodeMap, UserSetId userSetId);

  GENIRANGER_API UserSetId getId() const { return mUserSetId; }
  GENIRANGER_API std::string getName() const { return toString(mUserSetId); }

  /** \return the description string for the user set */
  GENIRANGER_API std::string getDescription() const;

  /** Sets the description string for the user set. */
  GENIRANGER_API void setDescription(const std::string& description);

  /** \return true if the user sets exists on the device. */
  GENIRANGER_API bool exists() const;

  /**
   * Loads the configuration from the user set and makes it active.
   *
   * \throw GenIRangerException if the user set doesn't exist or needs migration
   */
  GENIRANGER_API void load();

  /**
   * Saves the current configuration to the user set.
   *
   * \throw GenIRangerException if this is the Default user set
   */
  GENIRANGER_API void save();

  /**
   * \return the user set as complete configuration in CSV format. The same
   *         format is used for import and export.
   * \see \ref ConnectedDevice::importParameters
   * \throw GenIRangerException if this is the Default user set
   */
  GENIRANGER_API std::string retrieveFromCamera() const;

  /**
   * Set this user set to be the one to use when the device is started.
   *
   * \throw GenIRangerException if the user set doesn't exist or needs migration
   */
  GENIRANGER_API void useAtStartup();

  /**
   * \return true if the user set needs to be migrated to the current firmware
   *         version.
   */
  GENIRANGER_API bool needsMigration() const;

  /**
   * Migrates the user set to the current firmware version. The import mechanism
   * is used to re-import the configuration to the device and thus the returned
   * ConfigurationResult may look like importing a parameter file created for
   * another firmware version.
   *
   * \return a ConfigurationResult. The result may have the status invalid even
   *         though the migration resulted in a valid configuration, this will
   *         happen if an old configuration parameter is no longer available for
   *         the current firmware.
   * \throw GenIRangerException if the migration results in configuration that
   *        is not valid
   */
  GENIRANGER_API ConfigurationResult migrate();

private:
  void checkExistsAndMigrated(const std::string& messageSuffix) const;

private:
  NodeMap mDeviceNodeMap;
  UserSetId mUserSetId;
};

}
