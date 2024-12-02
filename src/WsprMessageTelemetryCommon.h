#pragma once

#include <array>
#include <cstring>

#include "WsprMessageRegularType1.h"


class WsprMessageTelemetryCommon
: public WsprMessageRegularType1
{
public:

    WsprMessageTelemetryCommon()
    {
        Reset();
    }

    void Reset()
    {
        WsprMessageRegularType1::Reset();

        id13_ = { "00" };
    }

    // 00 through 09, 10 through 19, Q0 through Q9
    bool SetId13(const char *id13)
    {
        bool retVal = false;

        if (id13 && strlen(id13) == 2)
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

    // 00 through 09, 10 through 19, Q0 through Q9
    const char *GetId13() const
    {
        return (const char *)id13_.data();
    }


private:

    std::array<char, 3> id13_;
};