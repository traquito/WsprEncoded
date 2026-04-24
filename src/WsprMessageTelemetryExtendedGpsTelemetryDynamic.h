#pragma once

#include "WsprMessageTelemetryExtendedCommonDynamic.h"
#include "WsprMessageTelemetryExtendedGpsTelemetryBase.h"


class WsprMessageTelemetryExtendedGpsTelemetry
: public WsprMessageTelemetryExtendedGpsTelemetryBase<WsprMessageTelemetryExtendedCommonDynamic<4>, 4>
{
};
