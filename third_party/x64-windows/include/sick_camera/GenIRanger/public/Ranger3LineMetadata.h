// Copyright 2018-2020 SICK AG. All rights reserved.

#pragma once

#include <cstdint>

/** Container for metadata per scan line. */
struct Ranger3LineMetadata
{
    /** Timestamp in nanoseconds. */
    uint64_t timestamp = 0;
    /**
     * The encoder counter's value when the profile exposure is triggered for each
     * scan line included in the payload.
     */
    int32_t encoderValue = 0;

    /**
     * 1 if a line trigger has been requested before the sensor was actually ready
     * for a new trigger.
     */
    uint8_t overtriggerCount = 0;
    /** 1 if frame trigger pin (IO pin 1) is high, otherwise 0. */
    bool frameTriggerActive = false;
    /** 1 if line trigger pin (IO pin 2) is high, otherwise 0. */
    bool lineTriggerActive = false;
    /** 1 if Encoder Reset (IO pin 3) is high, otherwise 0. */
    bool encoderResetActive = false;
    /** Value 0 (for low) or 1 (for high) at encoder channel A. */
    bool encoderA = false;
    /** Value 0 (for low) or 1 (for high) at encoder channel B. */
    bool encoderB = false;

    bool operator==(const Ranger3LineMetadata& rhs) const
    {
    return timestamp == rhs.timestamp && encoderValue == rhs.encoderValue
            && overtriggerCount == rhs.overtriggerCount
            && frameTriggerActive == rhs.frameTriggerActive
            && lineTriggerActive == rhs.lineTriggerActive
            && encoderResetActive == rhs.encoderResetActive
            && encoderA == rhs.encoderA && encoderB == rhs.encoderB;
    }

    bool operator!=(const Ranger3LineMetadata& rhs) const { return !operator==(rhs); }
};
