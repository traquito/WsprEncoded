#include <cmath>
#include <iostream>
using namespace std;

#include "WsprMessageTelemetryExtendedTrackerTelemetry.h"


bool TestHdrTypeIsRestricted()
{
    bool retVal = true;

    WsprMessageTelemetryExtendedTrackerTelemetry msg;

    bool ok = msg.Set("HdrType", 0);

    retVal &= (ok == false);

    cout << "TestHdrTypeIsRestricted: " << retVal << endl;

    return retVal;
}

bool TestDefaults()
{
    WsprMessageTelemetryExtendedTrackerTelemetry msg;

    bool retVal =
        msg.GetFieldDefListLen() == 7 &&
        msg.GetHdrType() == WsprMessageTelemetryExtendedTrackerTelemetry::HdrType::TRACKER_TELEMETRY &&
        msg.GetId13Idx() == 0 &&
        msg.GetTempF() == -80 &&
        fabs(msg.GetVoltageV() - 2.7) < 0.0000001 &&
        msg.GetWindowSeqNo() == 1 &&
        msg.GetGpsLockType() == WsprMessageTelemetryExtendedTrackerTelemetry::GpsLockType::NO_LOCK &&
        msg.GetSubLatIdx() == 0 &&
        msg.GetSubLngIdx() == 0;

    cout << "TestDefaults: " << retVal << endl;

    return retVal;
}

bool TestEncodeDecode()
{
    WsprMessageTelemetryExtendedTrackerTelemetry msg1;
    msg1.SetId13("Q5");
    msg1.SetHdrSlot(4);
    msg1.SetId13Idx(3);
    msg1.SetTempF(23);
    msg1.SetVoltageV(4.12);
    msg1.SetWindowSeqNo(8);
    msg1.SetGpsLockType(WsprMessageTelemetryExtendedTrackerTelemetry::GpsLockType::LOCATION_LOCK);
    msg1.SetSubLatIdx(8);
    msg1.SetSubLngIdx(20);
    msg1.Encode();

    WsprMessageTelemetryExtendedTrackerTelemetry msg2;
    msg2.SetCallsign(msg1.GetCallsign());
    msg2.SetGrid4(msg1.GetGrid4());
    msg2.SetPowerDbm(msg1.GetPowerDbm());
    bool decodeOk = msg2.Decode();

    bool retVal =
        decodeOk == true &&
        msg2.GetHdrType() == WsprMessageTelemetryExtendedTrackerTelemetry::HdrType::TRACKER_TELEMETRY &&
        msg2.GetHdrSlot() == 4 &&
        msg2.GetId13Idx() == 3 &&
        msg2.GetTempF() == 25 &&
        fabs(msg2.GetVoltageV() - 4.12) < 0.0000001 &&
        msg2.GetWindowSeqNo() == 2 &&
        msg2.GetGpsLockType() == WsprMessageTelemetryExtendedTrackerTelemetry::GpsLockType::LOCATION_LOCK &&
        msg2.GetSubLatIdx() == 8 &&
        msg2.GetSubLngIdx() == 20;

    cout << "TestEncodeDecode: " << retVal << endl;

    return retVal;
}

bool TestLocationHelpers()
{
    WsprMessageTelemetryExtendedTrackerTelemetry msg;
    WsprMessageTelemetryExtendedTrackerTelemetry::Location location;

    bool encodeOk = msg.EncodeLocationToFieldValues(92, 77, 40.742, -70.032, location);

    double decodedLatitude = 0.0;
    double decodedLongitude = 0.0;
    bool decodeOk = msg.DecodeFieldValuesToLocation(92, 77, location.subLatIdx, location.subLngIdx, decodedLatitude, decodedLongitude);

    bool retVal =
        encodeOk == true &&
        location.subLatIdx == 8 &&
        location.subLngIdx == 20 &&
        decodeOk == true &&
        decodedLatitude > 40.74 && decodedLatitude < 40.75 &&
        decodedLongitude > -70.04 && decodedLongitude < -70.00;

    cout << "TestLocationHelpers: " << retVal << endl;

    return retVal;
}

bool TestWindowNormalization()
{
    bool retVal =
        WsprMessageTelemetryExtendedTrackerTelemetry::NormalizeWindowSeqNo(1) == 1 &&
        WsprMessageTelemetryExtendedTrackerTelemetry::NormalizeWindowSeqNo(6) == 6 &&
        WsprMessageTelemetryExtendedTrackerTelemetry::NormalizeWindowSeqNo(7) == 1 &&
        WsprMessageTelemetryExtendedTrackerTelemetry::NormalizeWindowSeqNo(0) == 6;

    cout << "TestWindowNormalization: " << retVal << endl;

    return retVal;
}

int main()
{
    bool retVal = true;

    retVal &= TestHdrTypeIsRestricted();
    retVal &= TestDefaults();
    retVal &= TestEncodeDecode();
    retVal &= TestLocationHelpers();
    retVal &= TestWindowNormalization();

    return !retVal;
}
