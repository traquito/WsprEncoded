#pragma once

#include <array>
#include <cmath>
#include <cstring>

#include "WsprMessageRegularType1.h"


class WsprMessageTelemetryBasic
: public WsprMessageRegularType1
{
public:

    WsprMessageTelemetryBasic()
    {
        Reset();
    }

    void Reset()
    {
        WsprMessageRegularType1::Reset();

        grid56_             = { "AA" };
        altitudeMeters_     = 0;
        temperatureCelsius_ = 0;
        voltageVolts_       = 3.3;
        speedKnots_         = 0;
        gpsIsValid_         = false;

        id13_ = { "00" };
    }

    /////////////////////////////////////////////////////////////////
    // Telemetry Setter / Getter Interface
    /////////////////////////////////////////////////////////////////

    bool SetGrid56(const char *grid56)
    {
        bool retVal = false;

        if (grid56)
        {
            if (strlen(grid56) == 2)
            {
                retVal = true;

                char grid5 = grid56[0];
                char grid6 = grid56[1];

                if      (grid5 < 'A') { grid5 = 'A'; retVal = false;}
                else if (grid5 > 'X') { grid5 = 'X'; retVal = false;}

                if      (grid6 < 'A') { grid6 = 'A'; retVal = false;}
                else if (grid6 > 'X') { grid6 = 'X'; retVal = false;}

                grid56_[0] = grid5;
                grid56_[1] = grid6;
            }
        }

        return retVal;
    }

    const char *GetGrid56() const
    {
        return (const char *)grid56_.data();
    }

    bool SetAltitudeMeters(int32_t altitudeMeters)
    {
        bool retVal = true;

        if      (altitudeMeters < 0)     { altitudeMeters = 0;     retVal = false; }
        else if (altitudeMeters > 21340) { altitudeMeters = 21340; retVal = false; }

        altitudeMeters_ = altitudeMeters;

        return retVal;
    }

    uint16_t GetAltitudeMeters() const
    {
        return altitudeMeters_;
    }

    bool SetTemperatureCelsius(int32_t temperatureCelsius)
    {
        bool retVal = true;

        if      (temperatureCelsius < -50) { temperatureCelsius = -50; retVal = false; }
        else if (temperatureCelsius >  39) { temperatureCelsius =  39; retVal = false; }

        temperatureCelsius_ = temperatureCelsius;

        return retVal;
    }

    int8_t GetTemperatureCelsius() const
    {
        return temperatureCelsius_;
    }

    bool SetVoltageVolts(double voltageVolts)
    {
        bool retVal = true;

        if      (voltageVolts < 3.00) { voltageVolts = 3.00; retVal = false; }
        else if (voltageVolts > 4.95) { voltageVolts = 4.95; retVal = false; }

        voltageVolts_ = voltageVolts;

        return retVal;
    }

    double GetVoltageVolts() const
    {
        return voltageVolts_;
    }

    bool SetSpeedKnots(int32_t speedKnots)
    {
        bool retVal = true;

        if      (speedKnots <  0) { speedKnots =  0; retVal = false; }
        else if (speedKnots > 82) { speedKnots = 82; retVal = false; }

        speedKnots_ = speedKnots;

        return retVal;
    }

    uint8_t GetSpeedKnots() const
    {
        return speedKnots_;
    }

    bool SetGpsIsValid(bool gpsValid)
    {
        gpsIsValid_ = gpsValid;

        return true;
    }

    bool GetGpsIsValid() const
    {
        return gpsIsValid_;
    }


    /////////////////////////////////////////////////////////////////
    // Encode / Decode Interface
    /////////////////////////////////////////////////////////////////

    bool SetId13(const char *id13)
    {
        bool retVal = false;

        if (strlen(id13) == 2)
        {
            char id1 = id13[0];
            char id3 = id13[1];

            if ((id1 == '0' || id1 == '1' || id1 == 'Q') &&
               ('0' <= id3 && id3 <= '9'))
            {
                id13_[0] = id1;
                id13_[1] = id3;

                retVal = true;
            }
        }

        return retVal;
    }

    const char *GetId13() const
    {
        return (const char *)id13_.data();
    }

    void Encode()
    {
        EncodeCallsign();
        EncodeGridPower();
    }

    bool Decode()
    {
        bool retVal = true;

        retVal &= DecodeU4BCall();
        retVal &= DecodeU4BGridPower();

        return retVal;
    }


private:

    /////////////////////////////////////////////////////////////////
    // Encoded/Decode Utilities
    /////////////////////////////////////////////////////////////////

    static char EncodeBase36(uint8_t val)
    {
        char retVal;

        if (val < 10)
        {
            retVal = '0' + val;
        }
        else
        {
            retVal = 'A' + (val - 10);
        }

        return retVal;
    }

    static uint8_t DecodeBase36(char c)
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

    static uint8_t DecodePowerDbmToNum(uint8_t powerDbm)
    {
        uint8_t retVal = 0;

        const auto &powerDbmList = Wspr::GetPowerDbmList();

        for (uint8_t i = 0; i < powerDbmList.size(); ++i)
        {
            if (powerDbm == powerDbmList[i])
            {
                retVal = i;
            }
        }

        return retVal;
    }


    /////////////////////////////////////////////////////////////////
    // Encode
    /////////////////////////////////////////////////////////////////

    void EncodeCallsign()
    {
        std::string callsign;

        // pick apart inputs
        char grid5 = grid56_[0];
        char grid6 = grid56_[1];

        // convert inputs into components of a big number
        uint8_t grid5Val = grid5 - 'A';
        uint8_t grid6Val = grid6 - 'A';

        uint16_t altFracM = std::round((double)altitudeMeters_ / 20);

        // convert inputs into a big number
        uint32_t val = 0;

        val *=   24; val += grid5Val;
        val *=   24; val += grid6Val;
        val *= 1068; val += altFracM;

        // extract
        uint8_t id6Val = val % 26; val = val / 26;
        uint8_t id5Val = val % 26; val = val / 26;
        uint8_t id4Val = val % 26; val = val / 26;
        uint8_t id2Val = val % 36; val = val / 36;

        // convert to encoded form
        char id2 = EncodeBase36(id2Val);
        char id4 = 'A' + id4Val;
        char id5 = 'A' + id5Val;
        char id6 = 'A' + id6Val;
        
        // store callsign
        static const uint8_t CALLSIGN_LEN = 6;
        char buf[CALLSIGN_LEN + 1] = { 0 };
        buf[0] = id13_[0];
        buf[1] = id2;
        buf[2] = id13_[1];
        buf[3] = id4;
        buf[4] = id5;
        buf[5] = id6;
        WsprMessageRegularType1::SetCallsign(buf);
    }

    void EncodeGridPower()
    {
        // map input presentations onto input radix (numbers within their stated range of possibilities)
        uint8_t tempCNum      = temperatureCelsius_ - -50;
        uint8_t voltageNum    = ((uint8_t)std::round(((voltageVolts_ * 100) - 300) / 5) + 20) % 40;
        uint8_t speedKnotsNum = std::round((double)speedKnots_ / 2.0);
        uint8_t gpsValidNum   = gpsIsValid_ ? 1 : 0;

        // convert inputs into a big number
        uint32_t val = 0;

        val *= 90; val += tempCNum;
        val *= 40; val += voltageNum;
        val *= 42; val += speedKnotsNum;
        val *=  2; val += gpsValidNum;
        val *=  2; val += 1;                // basic telemetry

        // extract
        uint8_t powerVal = val % 19; val = val / 19;
        uint8_t g4Val    = val % 10; val = val / 10;
        uint8_t g3Val    = val % 10; val = val / 10;
        uint8_t g2Val    = val % 18; val = val / 18;
        uint8_t g1Val    = val % 18; val = val / 18;

        // convert to encoded form
        char g1 = 'A' + g1Val;
        char g2 = 'A' + g2Val;
        char g3 = '0' + g3Val;
        char g4 = '0' + g4Val;

        // store grid
        static const uint8_t GRID4_LEN = 4;
        char buf[GRID4_LEN + 1] = { 0 };
        buf[0] = g1;
        buf[1] = g2;
        buf[2] = g3;
        buf[3] = g4;
        WsprMessageRegularType1::SetGrid4(buf);

        // store power
        uint8_t powerDbm = Wspr::GetPowerDbmList()[powerVal];
        WsprMessageRegularType1::SetPowerDbm(powerDbm);
    }


    /////////////////////////////////////////////////////////////////
    // Decode
    /////////////////////////////////////////////////////////////////

    bool DecodeU4BCall()
    {
        bool retVal = true;

        static const uint8_t CALLSIGN_DECODE_LEN = 6;

        const char *call = WsprMessageRegularType1::GetCallsign();

        if (strlen(call) == CALLSIGN_DECODE_LEN)
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

            // store grid56
            char grid5 = grid5Val + 'A';
            char grid6 = grid6Val + 'A';
            char grid56[3] = { 0 };
            grid56[0] = grid5;
            grid56[1] = grid6;
            retVal &= SetGrid56(grid56);

            // store altitudeMeters
            uint16_t altitudeMeters = altFracM * 20;
            retVal &= SetAltitudeMeters(altitudeMeters);
        }
        else
        {
            retVal = false;
        }

        return retVal;
    }

    // static tuple<uint8_t, int8_t, double, uint8_t, bool> 
    bool DecodeU4BGridPower()
    {
        bool retVal = true;

        const char *grid4 = WsprMessageRegularType1::GetGrid4();

        uint8_t g1Val = grid4[0] - 'A';
        uint8_t g2Val = grid4[1] - 'A';
        uint8_t g3Val = grid4[2] - '0';
        uint8_t g4Val = grid4[3] - '0';
        uint8_t powerVal = DecodePowerDbmToNum(WsprMessageRegularType1::GetPowerDbm());

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

        // can only decode basic telemetry
        if (telemetryId == 1)
        {
            int8_t  tempC      = -50 + (int8_t)tempCNum;
            double  voltage    = 3.0 + (((voltageNum + 20) % 40) * 0.05);
            uint8_t speedKnots = speedKnotsNum * 2;
            bool    gpsValid   = bit2;

            retVal &= SetTemperatureCelsius(tempC);
            retVal &= SetVoltageVolts(voltage);
            retVal &= SetSpeedKnots(speedKnots);
            retVal &= SetGpsIsValid(gpsValid);
        }
        else
        {
            retVal = false;
        }

        return retVal;
    }


private:

    std::array<char, 3> grid56_;
    uint16_t            altitudeMeters_;
    int8_t              temperatureCelsius_;
    double              voltageVolts_;
    uint8_t             speedKnots_;
    bool                gpsIsValid_;

    std::array<char, 3> id13_;
};

