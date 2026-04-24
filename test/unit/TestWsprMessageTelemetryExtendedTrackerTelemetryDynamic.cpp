#include "WsprMessageTelemetryExtendedTrackerTelemetryDynamic.h"


int main()
{
    WsprMessageTelemetryExtendedTrackerTelemetry<7> codec;

    return !(codec.GetHdrType() == WsprMessageTelemetryExtendedTrackerTelemetry<7>::HdrType::TRACKER_TELEMETRY);
}
