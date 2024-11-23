#pragma once

#include <cstdint>
#include <cmath>
#include <string>
#include <utility>

#include "WSPR.h"


inline uint8_t DecodeBase36(char c)
{
    uint8_t retVal = 0;

    uint8_t cVal = c;

    uint8_t aVal    = 'A';
    uint8_t zVal    = 'Z';
    uint8_t zeroVal = '0';

    if (aVal <= cVal && cVal <= zVal)
    {
        retVal = 10 + (cVal - aVal);
    }
    else
    {
        retVal = cVal - zeroVal;
    }

    return retVal;
}


inline uint8_t DecodePowerToNum(uint8_t powerDbm)
{
    uint8_t retVal = 0;

    const auto &powerDbmList = WSPR::GetPowerDbmList();

    for (uint8_t i = 0; i < powerDbmList.size(); ++i)
    {
        if (powerDbm == powerDbmList[i])
        {
            retVal = i;
        }
    }

    return retVal;
}


class WSPRMessageU4BDecoder
{
public:
    static pair<string, uint16_t> DecodeU4BCall(const string &call)
    {
        // break call down
        uint8_t id2 = call[1];
        uint8_t id4 = call[3];
        uint8_t id5 = call[4];
        uint8_t id6 = call[5];

        // convert to values which are offset from 'A'
        uint8_t id2Val = DecodeBase36(id2);
        uint8_t id4Val = id4 - 'A';
        uint8_t id5Val = id5 - 'A';
        uint8_t id6Val = id6 - 'A';

        // integer value to use to decode
        uint32_t val = 0;

        // combine values into single integer
        val *= 36; val += id2Val;
        val *= 26; val += id4Val;   // spaces aren't used, so 26 not 27
        val *= 26; val += id5Val;   // spaces aren't used, so 26 not 27
        val *= 26; val += id6Val;   // spaces aren't used, so 26 not 27

        // extract values
        uint16_t altFracM = val % 1068; val /= 1068;
        uint8_t  grid6Val = val %   24; val /=   24;
        uint8_t  grid5Val = val %   24; val /=   24;

        uint16_t altM   = altFracM * 20;
        char     grid6  = grid6Val + 'A';
        char     grid5  = grid5Val + 'A';
        string   grid56 = string{grid5, grid6};

        return { grid56, altM };
    }

    static tuple<uint8_t, int8_t, double, uint8_t, bool> DecodeU4BGridPower(string grid, uint8_t power)
    {
        uint8_t g1Val = grid[0] - 'A';
        uint8_t g2Val = grid[1] - 'A';
        uint8_t g3Val = grid[2] - '0';
        uint8_t g4Val = grid[3] - '0';
        uint8_t powerVal = DecodePowerToNum(power);

        uint32_t val = 0;
        
        val *= 18; val += g1Val;
        val *= 18; val += g2Val;
        val *= 10; val += g3Val;
        val *= 10; val += g4Val;
        val *= 19; val += powerVal;

        uint8_t telemetryId   = val %  2; val /=  2;
        uint8_t bit2          = val %  2; val /=  2;
        uint8_t speedKnotsNum = val % 42; val /= 42;
        uint8_t voltageNum    = val % 40; val /= 40;
        uint8_t tempCNum      = val % 90; val /= 90;

        // default return values in case bad input
        int8_t  tempC      = 0;
        double  voltage    = 3;
        uint8_t speedKnots = 0;
        bool    gpsValid   = false;

        if (telemetryId == 1)
        {
            gpsValid   = bit2;
            tempC      = -50 + (int8_t)tempCNum;
            voltage    = 3.0 + (((voltageNum + 20) % 40) * 0.05);
            speedKnots = speedKnotsNum * 2;
        }

        return { telemetryId == 1, tempC, voltage, speedKnots, gpsValid };
    }
};

