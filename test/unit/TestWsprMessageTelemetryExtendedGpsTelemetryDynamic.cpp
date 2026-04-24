#include "WsprMessageTelemetryExtendedGpsTelemetryDynamic.h"


int main()
{
    WsprMessageTelemetryExtendedGpsTelemetry codec;

    return !(codec.GetHdrType() == WsprMessageTelemetryExtendedGpsTelemetry::HdrType::GPS_TELEMETRY);
}
