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

private:

    // hide header setting
    using Base::SetCanSetHdrType;
};

