// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include <cstdint>

namespace geniranger {

#ifndef SWIG
/**
 * All custom commands are at a value 1000 or higher, we have selected 3300 as
 * base.
 */
const uint32_t CUSTOM_CMN_ID_BASE = 3300;
#endif

/**
 * Definition of extensions to the Part Info Commands. Used to get fields in the
 * underlying GenTL buffer part that are not defined in GenTL 1.5
 */
enum BUFFER_PART_CUSTOM_INFO_CMD_LIST
{
  /* UINT64 Intended to carry GEV's region_id until introduced in GenTL. */
  BUFFER_PART_CUSTOM_INFO_REGION_ID = CUSTOM_CMN_ID_BASE + 0,
  /* UINT64 Intended to carry GEV's data_purpose_id until in GenTL. */
  BUFFER_PART_CUSTOM_INFO_DATA_PURPOSE_ID = CUSTOM_CMN_ID_BASE + 1
};
}
