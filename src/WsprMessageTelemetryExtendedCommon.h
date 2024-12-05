#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "WsprMessageTelemetryCommon.h"


/////////////////////////////////////////////////////////////////
// This is a template class, you must specify the number of
// fields you intend to work with.
//
// The maximum theoretical number of fields is 29 1-bit fields.
//
// The total bitspace to configure fields within is 29.5 bits.
/////////////////////////////////////////////////////////////////
template <uint8_t FIELD_COUNT = 29>
class WsprMessageTelemetryExtendedCommon
: public WsprMessageTelemetryCommon
{
public:

    WsprMessageTelemetryExtendedCommon()
    {
        ResetEverything();
    }

    // Reset field values, but not field definitions
    void Reset()
    {
        WsprMessageTelemetryCommon::Reset();

        // reset default field value for user-defined fields
        for (int i = 0; i < fieldDefUserDefinedListIdx_; ++i)
        {
            fieldDefUserDefinedList_[i].value = fieldDefUserDefinedList_[i].lowValue;
        }

        // reset header values
        fieldDefHeaderList_[0].value = 0;
        fieldDefHeaderList_[1].value = 0;
        fieldDefHeaderList_[2].value = 0;
        fieldDefHeaderList_[3].value = 0;
    }

    // Reset field definitions and values
    void ResetEverything()
    {
        WsprMessageTelemetryCommon::Reset();

        fieldDefUserDefinedList_ = std::array<FieldDef, FIELD_COUNT>{};
        fieldDefUserDefinedListIdx_ = 0;

        numBitsSum_ = 0.0;

        fieldDefHeaderList_ = std::array<FieldDef, 4>{{
            {
                .name      = "HdrTelemetryType",
                .lowValue  = 0,
                .highValue = 1,
                .stepSize  = 1,
                .numValues = 2,
                .numBits   = 1,
                .value     = 0, // Extended Telemetry
            },
            {
                .name      = "HdrRESERVED",
                .lowValue  = 0,
                .highValue = 3,
                .stepSize  = 1,
                .numValues = 4,
                .numBits   = 2,
                .value     = 0,
            },
            {
                .name      = "HdrSlot",
                .lowValue  = 0,
                .highValue = 3,
                .stepSize  = 1,
                .numValues = 4,
                .numBits   = 2,
                .value     = 0,
            },
            {
                .name      = "HdrType",
                .lowValue  = 0,
                .highValue = 15,
                .stepSize  = 1,
                .numValues = 16,
                .numBits   = 4,
                .value     = 0, // User-Defined
            },
        }};

        canSetHdrType_ = true;
    }


    /////////////////////////////////////////////////////////////////
    // User-Defined Field Definitions, Setters, Getters
    /////////////////////////////////////////////////////////////////

    // Set up this object to know about named fields with a given
    // numeric range and resolution (step size)
    //
    // Values will be clamped between lowValue - highValue, inclusive.
    // Negative, zero, positive, and decimal values are all supported.
    // 
    // See Set() for details on rounding.
    //
    // The initial value of a defined field will be the specified lowValue
    //
    // Returns true if field is accepted
    // Returns false if field is rejected
    //
    // A field will be rejected due to:
    // - The template-specified number of fields have already been configured
    // - The field name is a nullptr
    // - The field already exists
    // - lowValue >= highValue
    // - stepSize <= 0
    // - The stepSize does not evenly divide the range between lowValue and highValue
    // - The field size exceeds the sum total capacity of 29.5 bits along with other fields
    //   or by itself
    bool DefineField(const char *fieldName,
                     double      lowValue,
                     double      highValue,
                     double      stepSize)
    {
        bool retVal = true;

        if (CanFitOneMore() == false)
        {
            retVal = false;
        }
        else if (fieldName == nullptr)
        {
            retVal = false;
        }
        else if (FieldDefExists(fieldName))
        {
            retVal = false;
        }
        else if (lowValue >= highValue)
        {
            retVal = false;
        }
        else if (stepSize <= 0)
        {
            retVal = false;
        }
        else
        {
            // is there a integer-number number of divisions of the low-to-high
            // range when incremented by the step size?
            double stepCount = (highValue - lowValue) / stepSize;
            if (stepCount != (int)stepCount)
            {
                retVal = false;
            }
            else
            {
                FieldDef fd;

                // known to be an integer value as checked previously
                fd.numValues = (uint32_t)(((highValue - lowValue) / stepSize) + 1);

                // calc bits used by this field
                fd.numBits = std::log2(fd.numValues);

                // check if can fit
                if (numBitsSum_ + fd.numBits > MAX_BITS)
                {
                    retVal = false;
                }
                else
                {
                    // increment number of bits used by total set of fields
                    numBitsSum_ += fd.numBits;

                    // capture field def values
                    fd.name      = fieldName;
                    fd.lowValue  = lowValue;
                    fd.highValue = highValue;
                    fd.stepSize  = stepSize;

                    // set initial value to known-within-range
                    fd.value = lowValue;

                    // add to field def list
                    fieldDefUserDefinedList_[fieldDefUserDefinedListIdx_] = fd;
                    ++fieldDefUserDefinedListIdx_;
                }
            }
        }

        return retVal;
    }

    // Set the value of a configured field.
    //
    // The value parameter is a double to make accepting a wide range
    // of values easily settable, even if you do not intend to use
    // a floating point number.
    //
    // A value that is set is retained internally at that precise value,
    // and will be returned at that value with a subsequent Get().
    // 
    // When a field is encoded, the encoded wspr data will contain the
    // encoded value, which itself is rounded to the precision specified
    // in the field definition.
    //
    // Returns true on success.
    // Returns false on error.
    //
    // An error will occur when:
    // - The field is not defined
    bool Set(const char *fieldName, double value)
    {
        bool retVal = true;

        if (FieldDefExists(fieldName) && IsOkToSet(fieldName))
        {
            FieldDef &fd = *GetFieldDef(fieldName);

            if (value < fd.lowValue)
            {
                value = fd.lowValue;

                retVal = false;
            }
            else if (value > fd.highValue)
            {
                value = fd.highValue;

                retVal = false;
            }

            fd.value = value;
        }
        else
        {
            retVal = false;
        }

        return retVal;
    }

    // Get the value of a configured field.
    // 
    // When a field is Set() then Get(), the value which was Set()
    // will be returned by Get().
    //
    // When a Decode() operation occurs, the decoded values are overwritten
    // onto the field values, and will become the new value which is
    // returned by Get().
    //
    // Returns the field value on success.
    // Returns NAN on error.
    // - You must use std::isnan() to check for NAN, you cannot compare via == NAN
    //
    // An error will occur when:
    // - The field is not defined
    double Get(const char *fieldName)
    {
        double retVal = NAN;

        if (FieldDefExists(fieldName))
        {
            FieldDef &fd = *GetFieldDef(fieldName);

            retVal = fd.value;
        }

        return retVal;
    }


    /////////////////////////////////////////////////////////////////
    // User-Defined Field Definitions, Setters, Getters
    /////////////////////////////////////////////////////////////////

    // Read the default HdrTelemetryType, or, read the value
    // which was set from Decode().
    uint8_t GetHdrTelemetryType()
    {
        return (uint8_t)Get("HdrTelemetryType");
    }

    // Read the default HdrRESERVED, or, read the value
    // which was set from Decode().
    uint8_t GetHdrRESERVED()
    {
        return (uint8_t)Get("HdrRESERVED");
    }

    // Set the Extended Telemetry HdrSlot value.
    //
    // This field associates the encoded telemetry with the Regular
    // message sent before it at the start minute associated with
    // the channel being transmitted on.
    //
    // In a given repeating 10-minute cycle, starting on the
    // start minute, which is the 0th minute, the slots are defined
    // as:
    // - start minute = [send Regular message]
    // - +2 min = slot 0
    // - +4 min = slot 1
    // - +6 min = slot 2
    // - +8 min = slot 3
    void SetHdrSlot(uint8_t val)
    {
        Set("HdrSlot", val);
    }

    // Read the default HdrSlot, the previously SetHdrSlot() slot number,
    // or, read the slot number which was set from Decode().
    uint8_t GetHdrSlot()
    {
        return (uint8_t)Get("HdrSlot");
    }

    enum class HdrType : uint8_t
    {
        USER_DEFINED = 0,
        RESERVED = 15,
    };

    void SetCanSetHdrType(bool val)
    {
        canSetHdrType_ = val;
    }

    // Read the default HdrType, or, read the value
    // which was set from Decode().
    uint8_t GetHdrType()
    {
        return (uint8_t)Get("HdrType");
    }


    /////////////////////////////////////////////////////////////////
    // Encode / Decode
    /////////////////////////////////////////////////////////////////

    // Encode the values of the defined fields into a set of
    // encoded WSPR Type 1 message fields (callsign, grid4, powerDbm).
    // This overwrites the Type 1 message fields.
    //
    // The functions GetCallsign(), GetGrid4(), and GetPowerDbm() will
    // subsequently return the encoded values for those fields.
    void Encode()
    {
        // create big number
        uint64_t val = 0;

        // create packing logic that is the same between header and application fields
        auto PackFields = [](uint64_t &val, FieldDef *fieldDefList, uint8_t fieldDefListLen) {
            for (int i = fieldDefListLen - 1; i >= 0; --i)
            {
                // get field def
                FieldDef &fd = fieldDefList[i];

                // calculate field number for packing
                uint32_t fieldNumber = (uint32_t)std::round((fd.value - fd.lowValue) / fd.stepSize);

                // pack
                val *= fd.numValues; val += fieldNumber;
            }
        };

        // pack application fields into big number in reverse-definition order
        PackFields(val, static_cast<FieldDef *>(fieldDefUserDefinedList_.data()), fieldDefUserDefinedListIdx_);

        // pack header fields into message in reverse-definition order
        PackFields(val, static_cast<FieldDef *>(fieldDefHeaderList_.data()), fieldDefHeaderList_.size());

        // encode into power
        uint8_t powerVal = val % 19; val /= 19;
        uint8_t powerDbm = Wspr::GetPowerDbmList()[powerVal];

        // encode into grid
        uint8_t g4Val = val % 10; val /= 10;
        uint8_t g3Val = val % 10; val /= 10;
        uint8_t g2Val = val % 18; val /= 18;
        uint8_t g1Val = val % 18; val /= 18;

        char g1 = 'A' + g1Val;
        char g2 = 'A' + g2Val;
        char g3 = '0' + g3Val;
        char g4 = '0' + g4Val;

        static const uint8_t GRID4_LEN = 4;
        char grid4[GRID4_LEN + 1] = { 0 };
        grid4[0] = g1;
        grid4[1] = g2;
        grid4[2] = g3;
        grid4[3] = g4;

        // encode into callsign
        uint8_t id6Val = val % 26; val /= 26;
        uint8_t id5Val = val % 26; val /= 26;
        uint8_t id4Val = val % 26; val /= 26;
        uint8_t id2Val = val % 36; val /= 36;

        char id2 = WsprMessageTelemetryCommon::EncodeBase36(id2Val);
        char id4 = 'A' + id4Val;
        char id5 = 'A' + id5Val;
        char id6 = 'A' + id6Val;

        static const uint8_t CALLSIGN_LEN = 6;
        char callsign[CALLSIGN_LEN + 1] = { 0 };
        callsign[0] = WsprMessageTelemetryCommon::GetId13()[0];
        callsign[1] = id2;
        callsign[2] = WsprMessageTelemetryCommon::GetId13()[1];
        callsign[3] = id4;
        callsign[4] = id5;
        callsign[5] = id6;

        // capture results
        WsprMessageRegularType1::SetCallsign(callsign);
        WsprMessageRegularType1::SetGrid4(grid4);
        WsprMessageRegularType1::SetPowerDbm(powerDbm);
    }

    // Decode the values of the WSPR Type 1 message fields that were
    // set by using SetCallsign(), SetGrid4(), and SetPowerDbm().
    // This overwrites every defined field value and header field value.
    //
    // Returns true on success.
    // Returns false on error.
    //
    // An error will occur when:
    // - The HdrTelemetryType is not ExtendedTelemetry
    // - The HdrRESERVED is not 0
    //
    // Even when Decode returns an error, Get() will still return the
    // field and header values which were decoded.
    //
    // The decoded field values are stored internally and are retrieved
    // by using Get().
    bool Decode()
    {
        bool retVal = true;

        // pull in inputs
        const char *callsign = WsprMessageRegularType1::GetCallsign();
        const char *grid4    = WsprMessageRegularType1::GetGrid4();
        uint8_t     powerDbm = WsprMessageRegularType1::GetPowerDbm();

        // break callsign down
        uint8_t id2Val = WsprMessageTelemetryCommon::DecodeBase36(callsign[1]);
        uint8_t id4Val = callsign[3] - 'A';
        uint8_t id5Val = callsign[4] - 'A';
        uint8_t id6Val = callsign[5] - 'A';

        // break grid down
        uint8_t g1Val = grid4[0] - 'A';
        uint8_t g2Val = grid4[1] - 'A';
        uint8_t g3Val = grid4[2] - '0';
        uint8_t g4Val = grid4[3] - '0';

        // break power down
        uint8_t powerVal = WsprMessageTelemetryCommon::DecodePowerDbmToNum(powerDbm);

        // create big number
        uint64_t val = 0;
        val *= 36; val += id2Val;
        val *= 26; val += id4Val;   // spaces aren't used, so 26 not 27
        val *= 26; val += id5Val;   // spaces aren't used, so 26 not 27
        val *= 26; val += id6Val;   // spaces aren't used, so 26 not 27
        val *= 18; val += g1Val;
        val *= 18; val += g2Val;
        val *= 10; val += g3Val;
        val *= 10; val += g4Val;
        val *= 19; val += powerVal;

        // create unpacking logic that is the same between header and application fields
        auto UnpackFields = [](uint64_t &val, FieldDef *fieldDefList, uint8_t fieldDefListLen) {
            for (int i = 0; i < fieldDefListLen; ++i)
            {
                // get field def
                FieldDef &fd = fieldDefList[i];

                // calculate field number
                uint32_t fieldNumber = val % fd.numValues;

                // set field value
                fd.value = fd.lowValue + (fieldNumber * fd.stepSize);

                // shed
                val /= fd.numValues;
            }
        };

        // unpack header fields
        UnpackFields(val, static_cast<FieldDef *>(fieldDefHeaderList_.data()), fieldDefHeaderList_.size());

        // unpack application fields
        UnpackFields(val, static_cast<FieldDef *>(fieldDefUserDefinedList_.data()), fieldDefUserDefinedListIdx_);

        retVal = GetHdrTelemetryType() == 0 && GetHdrRESERVED() == 0;

        return retVal;
    }


private:

    static constexpr double MAX_BITS = 29.5;

    struct FieldDef
    {
        const char *name;

        // field configuration
        double lowValue;
        double highValue;
        double stepSize;

        // calculated properties of configuration
        uint32_t numValues;
        double   numBits;

        // dynamic value clamped to configuration
        double value;
    };

    FieldDef *GetFieldDefFrom(const char *fieldName, FieldDef *fieldDefList, uint8_t fieldDefListLen)
    {
        FieldDef *retVal = nullptr;

        if (fieldName)
        {
            for (int i = 0; i < fieldDefListLen; ++i)
            {
                FieldDef &fd = fieldDefList[i];

                if (strcmp(fieldName, fd.name) == 0)
                {
                    retVal = &fd;
                    
                    break;
                }
            }
        }

        return retVal;
    }

    FieldDef *GetFieldDefHeader(const char *fieldName)
    {
        return GetFieldDefFrom(fieldName,
                               static_cast<FieldDef *>(fieldDefHeaderList_.data()),
                               fieldDefHeaderList_.size());
    }

    FieldDef *GetFieldDefUserDefined(const char *fieldName)
    {
        return GetFieldDefFrom(fieldName,
                               static_cast<FieldDef *>(fieldDefUserDefinedList_.data()),
                               fieldDefUserDefinedListIdx_);
    }

    FieldDef *GetFieldDef(const char *fieldName)
    {
        FieldDef *retVal = nullptr;

        retVal = GetFieldDefHeader(fieldName);

        if (retVal == nullptr)
        {
            retVal = GetFieldDefUserDefined(fieldName);
        }

        return retVal;
    }

    bool FieldDefExists(const char *fieldName)
    {
        return GetFieldDef(fieldName) != nullptr;
    }

    bool IsOkToSet(const char *fieldName)
    {
        bool retVal = true;

        if (fieldName)
        {
            // reject if field name is on the restricted list
            std::array<const char *, 3> restrictedFieldNameListArr = {
                "HdrTelemetryType",
                "HdrRESERVED",
                "HdrType",
            };

            const char **restrictedFieldNameList = reinterpret_cast<const char **>(restrictedFieldNameListArr.data());
            int restrictedFieldNameListLen = canSetHdrType_ ? 2 : 3;

            for (int i = 0; i < restrictedFieldNameListLen; ++i)
            {
                if (strcmp(fieldName, restrictedFieldNameList[i]) == 0)
                {
                    retVal = false;
                }
            }
        }
        else
        {
            retVal = false;
        }

        return retVal;
    }

    bool CanFitOneMore()
    {
        return fieldDefUserDefinedListIdx_ < fieldDefUserDefinedList_.size();
    }


private:

    std::array<FieldDef, FIELD_COUNT> fieldDefUserDefinedList_;
    uint16_t fieldDefUserDefinedListIdx_;

    double numBitsSum_;

    // never changes count, but values can change
    std::array<FieldDef, 4> fieldDefHeaderList_;

    bool canSetHdrType_;
};

