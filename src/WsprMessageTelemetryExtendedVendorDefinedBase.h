#pragma once

#include <cstdint>


template <typename WsprMessageTelemetryExtendedCommonType, uint8_t FIELD_COUNT = 29>
class WsprMessageTelemetryExtendedVendorDefinedBase
: public WsprMessageTelemetryExtendedCommonType
{
    using Base = WsprMessageTelemetryExtendedCommonType;

public:

    WsprMessageTelemetryExtendedVendorDefinedBase()
    {
        // don't allow this named class to change its protocol type
        Base::Set("HdrType", (uint8_t)Base::HdrType::VENDOR_DEFINED);
        Base::SetCanSetHdrType(false);
    }

    // Returns true on successful decode.
    // Returns false on error.
    //
    // An error will occur when:
    // - The HdrTelemetryType is not ExtendedTelemetry
    // - The HdrType is not of type VendorDefined
    bool Decode()
    {
        bool retVal = true;
        
        retVal &= Base::Decode();
        retVal &= Base::GetHdrType() == Base::HdrType::VENDOR_DEFINED;

        return retVal;
    }
    

private:

    // hide header setting
    using Base::SetCanSetHdrType;
};
