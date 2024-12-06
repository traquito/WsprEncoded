#pragma once

#include "WsprMessageTelemetryExtendedCommon.h"


template <uint8_t FIELD_COUNT = 29>
class WsprMessageTelemetryExtendedUserDefined
: public WsprMessageTelemetryExtendedCommon<FIELD_COUNT>
{
    using Base = WsprMessageTelemetryExtendedCommon<FIELD_COUNT>;

public:

    WsprMessageTelemetryExtendedUserDefined()
    {
        // don't allow this named class to change its protocol type
        Base::Set("HdrType", (uint8_t)Base::HdrType::USER_DEFINED);
        Base::SetCanSetHdrType(false);
    }

    // Returns true on successful decode.
    // Returns false on error.
    //
    // An error will occur when:
    // - The HdrTelemetryType is not ExtendedTelemetry
    // - The HdrRESERVED is not 0
    // - The HdrType is not of type UserDefined
    bool Decode()
    {
        bool retVal = true;
        
        retVal &= Base::Decode();
        retVal &= Base::GetHdrType() == Base::HdrType::USER_DEFINED;

        return retVal;
    }
    

private:

    // hide header setting
    using Base::SetCanSetHdrType;
};

