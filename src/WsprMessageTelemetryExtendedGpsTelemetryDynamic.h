#pragma once

#include "WsprMessageTelemetryExtendedCommonDynamic.h"
#include "WsprMessageTelemetryExtendedGpsTelemetryBase.h"


template <uint8_t FIELD_COUNT = 4>
class WsprMessageTelemetryExtendedGpsTelemetry
: public WsprMessageTelemetryExtendedGpsTelemetryBase<WsprMessageTelemetryExtendedCommonDynamic<FIELD_COUNT>, FIELD_COUNT>
{
};
