#include <iostream>
using namespace std;

#include "WsprMessageTelemetryExtendedGpsTelemetry.h"


bool TestDefaults()
{
    WsprMessageTelemetryExtendedGpsTelemetry msg;

    bool retVal =
        msg.GetFieldDefListLen() == 4 &&
        msg.GetHdrType() == WsprMessageTelemetryExtendedGpsTelemetry::HdrType::GPS_TELEMETRY &&
        msg.GetLatitudeIdx() == 0 &&
        msg.GetLongitudeIdx() == 0 &&
        msg.GetAltitudeFt() == 0 &&
        msg.GetSpeedMPH() == 0;

    cout << "TestDefaults: " << retVal << endl;

    return retVal;
}

bool TestEncodeDecode()
{
    WsprMessageTelemetryExtendedGpsTelemetry msg1;
    msg1.SetId13("Q5");
    msg1.SetHdrSlot(3);
    msg1.SetLatitudeIdx(92);
    msg1.SetLongitudeIdx(77);
    msg1.SetAltitudeFt(38500);
    msg1.SetSpeedMPH(143);
    msg1.Encode();

    WsprMessageTelemetryExtendedGpsTelemetry msg2;
    msg2.SetCallsign(msg1.GetCallsign());
    msg2.SetGrid4(msg1.GetGrid4());
    msg2.SetPowerDbm(msg1.GetPowerDbm());
    bool decodeOk = msg2.Decode();

    bool retVal =
        decodeOk == true &&
        msg2.GetHdrType() == WsprMessageTelemetryExtendedGpsTelemetry::HdrType::GPS_TELEMETRY &&
        msg2.GetHdrSlot() == 3 &&
        msg2.GetLatitudeIdx() == 92 &&
        msg2.GetLongitudeIdx() == 77 &&
        msg2.GetAltitudeFt() == 38475 &&
        msg2.GetSpeedMPH() == 142;

    cout << "TestEncodeDecode: " << retVal << endl;

    return retVal;
}

bool TestLocationHelpers()
{
    WsprMessageTelemetryExtendedGpsTelemetry msg;
    WsprMessageTelemetryExtendedGpsTelemetry::Location location;

    bool encodeOk = msg.EncodeLocationToFieldValues(40.742, -70.032, location);

    double decodedLatitude = 0.0;
    double decodedLongitude = 0.0;
    bool decodeOk = msg.DecodeFieldValuesToLocation(location.latitudeIdx, location.longitudeIdx, decodedLatitude, decodedLongitude);

    bool retVal =
        encodeOk == true &&
        location.latitudeIdx == 92 &&
        location.longitudeIdx == 77 &&
        decodeOk == true &&
        decodedLatitude > 41.10 && decodedLatitude < 41.11 &&
        decodedLongitude > -70.16 && decodedLongitude < -70.15;

    cout << "TestLocationHelpers: " << retVal << endl;

    return retVal;
}

int main()
{
    bool retVal = true;

    retVal &= TestDefaults();
    retVal &= TestEncodeDecode();
    retVal &= TestLocationHelpers();

    return !retVal;
}
