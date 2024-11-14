#include <cstdint>
#include <iostream>
#include <string>

#include "WSPR.h"
#include "WSPRMessage.h"
#include "WSPRMessageU4B.h"


/////////////////////////////////////////////////////////////////////
// Sad hack to get around cout not doing what you'd expect
// https://stackoverflow.com/questions/19562103/uint8-t-cant-be-printed-with-cout
/////////////////////////////////////////////////////////////////////

template <typename T>
const T MakeCoutWork(const T val) { return val; }
const unsigned int MakeCoutWork(const uint8_t val) { return val; }


/////////////////////////////////////////////////////////////////////
// Testing Scaffolding
/////////////////////////////////////////////////////////////////////

template <typename T1, typename T2>
void ErrExpecting(std::string name, const T1 actualVal, const T2 expectedVal)
{
    std::cout
        << "ERR: Expected " << name
        << " to have value \"" << MakeCoutWork(expectedVal) << "\""
        << ", but got \"" << MakeCoutWork(actualVal) << "\""
        << std::endl;
}

template <typename T1, typename T2>
bool TestEqual(std::string name, const T1 actualVal, const T2 expectedVal)
{
    bool ok = actualVal == expectedVal;

    if (ok == false)
    {
        ErrExpecting(name, actualVal, expectedVal);
    }

    return ok;
}


/////////////////////////////////////////////////////////////////////
// Tests
/////////////////////////////////////////////////////////////////////

bool TestChannelDetails()
{
    bool ok = true;

    std::string band    = "20m";
    uint16_t    channel = 368;

    WSPR::ChannelDetails cd = WSPR::GetChannelDetails(band, channel);

    ok &= TestEqual("cd.id1",  cd.id1,  '1');
    ok &= TestEqual("cd.id3",  cd.id3,  '8');
    ok &= TestEqual("cd.id13", cd.id13, "18");
    ok &= TestEqual("cd.min",  cd.min,  4);
    ok &= TestEqual("cd.freq", cd.freq, (uint32_t)14'097'060);

    return ok;
}

bool TestMessageU4B()
{
    bool ok = true;

    WSPRMessageU4B msgU4b;
    msgU4b.SetId13("Q5");
    msgU4b.SetGrid56("JM");
    msgU4b.SetAltM(5120);
    msgU4b.SetTempC(-5);
    msgU4b.SetVoltage(3.25);
    msgU4b.SetSpeedKnots(25);
    msgU4b.SetGpsValid(true);

    std::string  callsign = msgU4b.GetCallsign();
    std::string  grid4    = msgU4b.GetGrid();
    uint8_t      pwrDbm   = msgU4b.GetPowerDbm();

    ok &= TestEqual("callsign", callsign, "QD5WPK");
    ok &= TestEqual("grid4",    grid4,    "IR39");
    ok &= TestEqual("pwrDbm",   pwrDbm,   47);

    return ok;
}


/////////////////////////////////////////////////////////////////////
// Entrypoint
/////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    bool ok = true;

    ok &= TestChannelDetails();
    ok &= TestMessageU4B();

    return !ok;
}

