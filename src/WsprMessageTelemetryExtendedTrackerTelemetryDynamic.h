#pragma once

#include "WsprMessageTelemetryExtendedCommonDynamic.h"
#include "WsprMessageTelemetryExtendedTrackerTelemetryBase.h"


class WsprMessageTelemetryExtendedTrackerTelemetry
: public WsprMessageTelemetryExtendedTrackerTelemetryBase<WsprMessageTelemetryExtendedCommonDynamic<7>, 7>
{
};