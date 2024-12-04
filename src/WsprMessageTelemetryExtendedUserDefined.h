#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "WsprMessageTelemetryCommon.h"


// default template param set to max possible field count
template <uint8_t FIELD_COUNT = 29>
class WsprMessageTelemetryExtendedUserDefined
: public WsprMessageTelemetryCommon
{
public:

    WsprMessageTelemetryExtendedUserDefined()
    {
        ResetEverything();
    }

    // reset just values
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

    // reset field definitions and values
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
    }


    /////////////////////////////////////////////////////////////////
    // User-Defined Field Definitions, Setters, Getters
    /////////////////////////////////////////////////////////////////

    // Define User-Defined Fields
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

    // Set User-Defined Field
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

    // Get User-Defined Field
    // Can return NAN (must use std::isnan(), cannot compare via == NAN)
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

    bool SetHdrSlot(uint8_t val)
    {
        return Set("HdrSlot", val);
    }

    uint8_t GetHdrSlot()
    {
        double val = Get("HdrSlot");

        return (uint8_t)val;
    }

    enum class HdrType : uint8_t
    {
        USER_DEFINED = 0,
        RESERVED = 15,
    };

    bool SetHdrType(HdrType val)
    {
        return Set("HdrType", (uint8_t)val);
    }

    HdrType GetHdrType()
    {
        HdrType retVal = HdrType::USER_DEFINED;

        double val = Get("HdrType");

        if (val != NAN)
        {
            retVal = (HdrType)(uint8_t)val;
        }

        return retVal;
    }


    /////////////////////////////////////////////////////////////////
    // Encode / Decode
    /////////////////////////////////////////////////////////////////

    // Encode fields
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

    // Decode into fields
    void Decode()
    {
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
    }


private:

    static constexpr double MAX_BITS = 29.5;

    struct FieldDef
    {
        const char *name = "Undefined";

        // field configuration
        double lowValue  = 0.0;
        double highValue = 0.0;
        double stepSize  = 0.0;

        // calculated properties of configuration
        uint32_t numValues = 0;
        double   numBits   = 0.0;

        // dynamic value clamped to configuration
        double value = 0.0;
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
            std::array<const char *, 2> restrictedFieldNameList = {
                "HdrTelemetryType",
                "HdrRESERVED",
            };

            for (const char *restrictedFieldName : restrictedFieldNameList)
            {
                if (strcmp(fieldName, restrictedFieldName) == 0)
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
};


