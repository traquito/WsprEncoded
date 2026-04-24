#include "WsprMessageTelemetryExtendedTrackerTelemetryDynamic.h"


int main()
{
    WsprMessageTelemetryExtendedTrackerTelemetry codec;

    return !(codec.GetHdrType() == WsprMessageTelemetryExtendedTrackerTelemetry::HdrType::TRACKER_TELEMETRY);
}
