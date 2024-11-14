#pragma once

#include <cstdint>
#include <string>

#include "WSPR.h"


class WSPRMessage
{
public:

    bool SetCallsign(const std::string callsign)
    {
        bool retVal = false;

        if (WSPR::CallsignIsValid(callsign))
        {
            callsign_ = callsign;

            retVal = true;
        }

        return retVal;
    }

    std::string GetCallsign() const { return callsign_; }
    
    bool SetGrid(const std::string grid)
    {
        bool retVal = false;

        if (WSPR::Grid4IsValid(grid))
        {
            grid_ = grid;

            retVal = true;
        }

        return retVal;
    }

    std::string GetGrid() const { return grid_; }

    bool SetPowerDbm(uint8_t powerDbm)
    {
        bool retVal = false;

        // check values
        if (WSPR::PowerDbmInSet(powerDbm))
        {
            powerDbm_ = powerDbm;

            retVal = true;
        }

        return retVal;
    }

    uint8_t GetPowerDbm() const { return powerDbm_; }
    

private:

    std::string callsign_ = "0A0AAA";
    std::string grid_     = "AA00";
    uint8_t     powerDbm_ = 0;
};

