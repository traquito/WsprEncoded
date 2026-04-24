#pragma once

#include "WsprMessageTelemetryExtendedCommon.h"
#include "WsprMessageTelemetryExtendedGpsTelemetryBase.h"


class WsprMessageTelemetryExtendedGpsTelemetry
: public WsprMessageTelemetryExtendedGpsTelemetryBase<WsprMessageTelemetryExtendedCommon<4>, 4>
{
};
