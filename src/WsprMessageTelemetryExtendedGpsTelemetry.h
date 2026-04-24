#pragma once

#include "WsprMessageTelemetryExtendedCommon.h"
#include "WsprMessageTelemetryExtendedGpsTelemetryBase.h"


template <uint8_t FIELD_COUNT = 4>
class WsprMessageTelemetryExtendedGpsTelemetry
: public WsprMessageTelemetryExtendedGpsTelemetryBase<WsprMessageTelemetryExtendedCommon<FIELD_COUNT>, FIELD_COUNT>
{
};
