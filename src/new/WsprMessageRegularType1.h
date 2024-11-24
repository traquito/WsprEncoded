#pragma once

#include <cstdint>
#include <cstring>

#include "Wspr.h"
#include "WsprUtl.h"


class WsprMessageRegularType1
{
public:

    WsprMessageRegularType1()
    {
        Reset();
    }

    void Reset()
    {
        callsign_.Target(callsignBuf_, CALLSIGN_LEN_MAX + 1);
        callsign_.Set("0A0AAA");

        grid_.Target(gridBuf_, GRID4_LEN + 1);
        grid_.Set("AA00");
    }

    bool SetCallsign(const char *callsign)
    {
        bool retVal = false;

        if (CallsignIsValid(callsign))
        {
            callsign_.Set(callsign);

            retVal = true;
        }

        return retVal;
    }

    const char *GetCallsign() const
    {
        return callsign_.Get();
    }
    
    bool SetGrid(const char *grid)
    {
        bool retVal = false;

        if (Grid4IsValid(grid))
        {
            grid_.Set(grid);

            retVal = true;
        }

        return retVal;
    }

    const char *GetGrid() const
    {
        return grid_.Get();
    }

    bool SetPowerDbm(uint8_t powerDbm)
    {
        bool retVal = false;

        if (Wspr::PowerDbmInSet(powerDbm))
        {
            powerDbm_ = powerDbm;

            retVal = true;
        }

        return retVal;
    }

    uint8_t GetPowerDbm() const
    {
        return powerDbm_;
    }
    

private:

    static bool CallsignIsValid(const char *inputCallsign)
    {
        bool retVal = false;

        // pointer not null
        if (inputCallsign)
        {
            // temporary copy of string
            size_t len = strlen(inputCallsign);
            char buf[len + 1];
            WsprUtl::CString callsign((char *)buf, len + 1);
            callsign.Set(inputCallsign);

            bool isPaddedLeft  = callsign.IsPaddedLeft();
            bool isPaddedRight = callsign.IsPaddedRight();
            bool isUppercase   = callsign.IsUppercase();

            // check criteria
            if (isPaddedLeft == false &&
                isPaddedRight == false &&
                isUppercase == true &&
                len >= CALLSIGN_LEN_MIN &&
                len <= CALLSIGN_LEN_MAX)
            {
                retVal = true;
            }
        }

        return retVal;
    }

    static bool Grid4IsValid(const char *grid)
    {
        bool retVal = false;

        if (grid)
        {
            size_t len = strlen(grid);

            if (len == GRID4_LEN)
            {
                char g1 = grid[0];
                char g2 = grid[1];
                char g3 = grid[2];
                char g4 = grid[3];

                if ('A' <= g1 && g1 <= 'R' &&
                    'A' <= g2 && g2 <= 'R' &&
                    '0' <= g3 && g3 <= '9' &&
                    '0' <= g4 && g4 <= '9')
                {
                    retVal = true;
                }
            }
        }

        return retVal;
    }


private:

    static const uint8_t CALLSIGN_LEN_MAX = 6;
    static const uint8_t CALLSIGN_LEN_MIN = 4;
    char callsignBuf_[CALLSIGN_LEN_MAX + 1];
    WsprUtl::CString callsign_;

    static const uint8_t GRID4_LEN = 4;
    char gridBuf_[GRID4_LEN + 1];
    WsprUtl::CString grid_;

    uint8_t powerDbm_ = 0;
};

