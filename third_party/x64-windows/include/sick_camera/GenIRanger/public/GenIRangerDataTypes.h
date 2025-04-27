// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include <string>
#include <vector>

namespace geniranger {

/**
 * Represents the result of importing a configuration in CSV format or migrating
 * a user set to the current firmware version.
 *
 * The enum values are set explicitly and should not be changed, in order to
 * simplify customer support.
 */
enum class ConfigurationStatus
{
  /** Successful import or migration */
  OK = 0,

  /**
   * The imported or migrated configuration either was missing some parameters,
   * contained unknown parameters or both.
   */
  WARNING = 1000,

  /** Generic error */
  ERROR_GENERIC = 2000,
  /** The format version of the configuration doesn't match the expected. */
  ERROR_VERSION_MISMATCH = 2001,
  /** The configuration is empty, and thus missing format version number. */
  ERROR_EMPTY_CONFIGURATION = 2002,
  /** One or more parameters are duplicated in the configuration. */
  ERROR_DUPLICATE_PARAMETER = 2003,
  /** The applied configuration is invalid. */
  ERROR_INVALID_CONFIGURATION = 2004,
};

struct ConfigurationResult
{
public:
  ConfigurationResult(ConfigurationStatus status, const std::string& message)
    : status(status)
    , message(message)
  {
  }

  bool hasMissing() const { return !missingParameters.empty(); }
  bool hasUnknown() const { return !unknownParameters.empty(); }

public:
  ConfigurationStatus status;
  std::string message;

  std::vector<std::string> missingParameters;
  std::vector<std::string> unknownParameters;
};

}
