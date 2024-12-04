#include <iostream>
using namespace std;

#include "WsprMessageTelemetryExtendedUserDefined.h"


bool TestHdrTypeIsRestricted()
{
    bool retVal = true;

    WsprMessageTelemetryExtendedUserDefined msg;

    bool ok = msg.Set("HdrType", 1);

    retVal &= (ok == false);

    cout << "TestHdrTypeIsRestricted: " << retVal << endl;

    return retVal;
}

int main()
{
    bool retVal = true;

    retVal &= TestHdrTypeIsRestricted();

    return !retVal;
}