#pragma once

#include "WsprMessageTelemetryExtendedCommonDynamic.h"
#include "WsprMessageTelemetryExtendedTrackerTelemetryBase.h"


template <uint8_t FIELD_COUNT = 7>
class WsprMessageTelemetryExtendedTrackerTelemetry
: public WsprMessageTelemetryExtendedTrackerTelemetryBase<WsprMessageTelemetryExtendedCommonDynamic<FIELD_COUNT>, FIELD_COUNT>
{
};
