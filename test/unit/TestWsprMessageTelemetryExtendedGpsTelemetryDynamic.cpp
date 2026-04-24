#include "WsprMessageTelemetryExtendedGpsTelemetryDynamic.h"


int main()
{
    WsprMessageTelemetryExtendedGpsTelemetry<4> codec;

    return !(codec.GetHdrType() == WsprMessageTelemetryExtendedGpsTelemetry<4>::HdrType::GPS_TELEMETRY);
}
