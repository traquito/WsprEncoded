#pragma once

#include "WsprMessageTelemetryExtendedCommon.h"
#include "WsprMessageTelemetryExtendedTrackerTelemetryBase.h"


template <uint8_t FIELD_COUNT = 7>
class WsprMessageTelemetryExtendedTrackerTelemetry
: public WsprMessageTelemetryExtendedTrackerTelemetryBase<WsprMessageTelemetryExtendedCommon<FIELD_COUNT>, FIELD_COUNT>
{
};
