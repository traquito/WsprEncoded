#pragma once

#include "WsprMessageTelemetryExtendedCommon.h"
#include "WsprMessageTelemetryExtendedTrackerTelemetryBase.h"


class WsprMessageTelemetryExtendedTrackerTelemetry
: public WsprMessageTelemetryExtendedTrackerTelemetryBase<WsprMessageTelemetryExtendedCommon<7>, 7>
{
};