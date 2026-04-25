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
// The total bitspace to configure fields within is 29.180 bits.
/////////////////////////////////////////////////////////////////
template <uint8_t FIELD_COUNT = 29>
class WsprMessageTelemetryExtendedCommon
: public WsprMessageTelemetryCommon
{
public:

    static constexpr double MAX_BITS = 29.180;
    static constexpr uint8_t MAX_SEGMENT_COUNT = 8;

    struct SegmentDef
    {
        double lowValue = 0;
        double highValue = 0;
        double stepSize = 0;
        uint32_t numValues = 0;
    };

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

        // optional segmented encoding
        bool isSegmented = false;
        uint8_t segmentCount = 0;
        std::array<SegmentDef, MAX_SEGMENT_COUNT> segmentList{};

        // dynamic value clamped to configuration
        double value;
    };


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
        fieldDefFailReason_ = "";

        numBitsSum_ = 0.0;

        fieldDefHeaderList_ = std::array<FieldDef, 4>{};

        fieldDefHeaderList_[0].name = "HdrTelemetryType";
        fieldDefHeaderList_[0].lowValue = 0;
        fieldDefHeaderList_[0].highValue = 1;
        fieldDefHeaderList_[0].stepSize = 1;
        fieldDefHeaderList_[0].numValues = 2;
        fieldDefHeaderList_[0].numBits = 1;
        fieldDefHeaderList_[0].value = 0;

        fieldDefHeaderList_[1].name = "HdrRESERVED";
        fieldDefHeaderList_[1].lowValue = 0;
        fieldDefHeaderList_[1].highValue = 3;
        fieldDefHeaderList_[1].stepSize = 1;
        fieldDefHeaderList_[1].numValues = 4;
        fieldDefHeaderList_[1].numBits = 2;
        fieldDefHeaderList_[1].value = 0;

        fieldDefHeaderList_[2].name = "HdrType";
        fieldDefHeaderList_[2].lowValue = 0;
        fieldDefHeaderList_[2].highValue = 15;
        fieldDefHeaderList_[2].stepSize = 1;
        fieldDefHeaderList_[2].numValues = 16;
        fieldDefHeaderList_[2].numBits = 4;
        fieldDefHeaderList_[2].value = 0;

        fieldDefHeaderList_[3].name = "HdrSlot";
        fieldDefHeaderList_[3].lowValue = 0;
        fieldDefHeaderList_[3].highValue = 4;
        fieldDefHeaderList_[3].stepSize = 1;
        fieldDefHeaderList_[3].numValues = 5;
        fieldDefHeaderList_[3].numBits = std::log2(5);
        fieldDefHeaderList_[3].value = 0;

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
    // - lowValue, highValue, or stepSize is too precise (more than 4 decimal places of precision)
    // - lowValue >= highValue
    // - stepSize <= 0
    // - The stepSize does not evenly divide the range between lowValue and highValue
    // - The field size exceeds the sum total capacity of 29.180 bits along with other fields
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

            fieldDefFailReason_ = "Can not fit another field";
        }
        else if (fieldName == nullptr)
        {
            retVal = false;

            fieldDefFailReason_ = "Field name is nullptr";
        }
        else if (FieldDefExists(fieldName))
        {
            retVal = false;

            fieldDefFailReason_ = "Field already exists";
        }
        else if (FieldIsTooPrecise(lowValue))
        {
            retVal = false;

            fieldDefFailReason_ = "Low value is too precise";
        }
        else if (FieldIsTooPrecise(highValue))
        {
            retVal = false;

            fieldDefFailReason_ = "High value is too precise";
        }
        else if (FieldIsTooPrecise(stepSize))
        {
            retVal = false;

            fieldDefFailReason_ = "Step size is too precise";
        }
        else if (lowValue >= highValue)
        {
            retVal = false;

            fieldDefFailReason_ = "Low value >= High value";
        }
        else if (stepSize <= 0)
        {
            retVal = false;

            fieldDefFailReason_ = "Step size <= 0";
        }
        else
        {
            SegmentDef segmentDef;
            segmentDef.lowValue = lowValue;
            segmentDef.highValue = highValue;
            segmentDef.stepSize = stepSize;
            segmentDef.numValues = GetNumValues(lowValue, highValue, stepSize);

            if (segmentDef.numValues == 0)
            {
                retVal = false;

                fieldDefFailReason_ = "Step size does not evenly divide the low-to-high range";
            }
            else
            {
                FieldDef fd;

                fd.numValues = segmentDef.numValues;

                // calc bits used by this field
                fd.numBits = std::log2(fd.numValues);

                // check if can fit
                if (numBitsSum_ + fd.numBits > MAX_BITS)
                {
                    retVal = false;

                    fieldDefFailReason_ = "Field overflows available bits";
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
                    fd.segmentList[0] = segmentDef;
                    fd.segmentCount = 1;

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

    const char *GetDefineFieldErr() const
    {
        return fieldDefFailReason_;
    }

    uint16_t GetFieldDefListLen() const
    {
        return fieldDefUserDefinedListIdx_;
    }

    const std::array<FieldDef, FIELD_COUNT> &GetFieldDefList() const
    {
        return fieldDefUserDefinedList_;
    }

    const FieldDef *GetFieldDefByName(const char *fieldName) const
    {
        auto GetFieldDefFromConst = [](const char *fieldName, const FieldDef *fieldDefList, uint8_t fieldDefListLen) -> const FieldDef * {
            const FieldDef *retVal = nullptr;

            if (fieldName)
            {
                for (int i = 0; i < fieldDefListLen; ++i)
                {
                    const FieldDef &fd = fieldDefList[i];

                    if (strcmp(fieldName, fd.name) == 0)
                    {
                        retVal = &fd;
                        break;
                    }
                }
            }

            return retVal;
        };

        const FieldDef *retVal = GetFieldDefFromConst(fieldName,
                                                      static_cast<const FieldDef *>(fieldDefHeaderList_.data()),
                                                      fieldDefHeaderList_.size());

        if (retVal == nullptr)
        {
            retVal = GetFieldDefFromConst(fieldName,
                                          static_cast<const FieldDef *>(fieldDefUserDefinedList_.data()),
                                          fieldDefUserDefinedListIdx_);
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

            // do value type validation
            if (std::isnan(value))
            {
                value = fd.lowValue;

                retVal = false;
            }
            else if (std::isinf(value))
            {
                value = fd.lowValue;

                retVal = false;
            }

            // check for negative zero, which may still wind up getting clamped
            if (value == 0.0 && std::signbit(value))
            {
                value = 0.0;

                retVal = false;
            }

            // clamp to range
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
    // This field associates the encoded telemetry with the sender.
    //
    // In a given repeating 10-minute cycle, starting on the
    // start minute, which is the 0th minute, the slots are defined
    // as:
    // - start minute = slot 0
    // - +2 min       = slot 1
    // - +4 min       = slot 2
    // - +6 min       = slot 3
    // - +8 min       = slot 4
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
        TRACKER_TELEMETRY = 1,
        GPS_TELEMETRY = 2,
        VENDOR_DEFINED = 15,
    };

    static const char *GetHdrTypeName(HdrType val)
    {
        const char *retVal = "Unknown";

        switch (val)
        {
            case HdrType::USER_DEFINED:
                retVal = "UserDefined";
                break;
            case HdrType::TRACKER_TELEMETRY:
                retVal = "TrackerTelemetry";
                break;
            case HdrType::GPS_TELEMETRY:
                retVal = "GpsTelemetry";
                break;
            case HdrType::VENDOR_DEFINED:
                retVal = "VendorDefined";
                break;
            default:
                break;
        }

        return retVal;
    }

    bool DefineField(const char *fieldName,
                     const SegmentDef *segmentList,
                     uint8_t           segmentCount)
    {
        bool retVal = true;

        if (CanFitOneMore() == false)
        {
            retVal = false;

            fieldDefFailReason_ = "Can not fit another field";
        }
        else if (fieldName == nullptr)
        {
            retVal = false;

            fieldDefFailReason_ = "Field name is nullptr";
        }
        else if (FieldDefExists(fieldName))
        {
            retVal = false;

            fieldDefFailReason_ = "Field already exists";
        }
        else if (segmentList == nullptr || segmentCount == 0)
        {
            retVal = false;

            fieldDefFailReason_ = "Segment list is empty";
        }
        else if (segmentCount > MAX_SEGMENT_COUNT)
        {
            retVal = false;

            fieldDefFailReason_ = "Segment list exceeds supported size";
        }
        else
        {
            FieldDef fd{};
            fd.name = fieldName;
            fd.isSegmented = true;
            fd.segmentCount = segmentCount;

            for (uint8_t i = 0; i < segmentCount && retVal; ++i)
            {
                const SegmentDef &inputSegment = segmentList[i];

                if (FieldIsTooPrecise(inputSegment.lowValue))
                {
                    retVal = false;
                    fieldDefFailReason_ = "Low value is too precise";
                }
                else if (FieldIsTooPrecise(inputSegment.highValue))
                {
                    retVal = false;
                    fieldDefFailReason_ = "High value is too precise";
                }
                else if (FieldIsTooPrecise(inputSegment.stepSize))
                {
                    retVal = false;
                    fieldDefFailReason_ = "Step size is too precise";
                }
                else if (inputSegment.lowValue >= inputSegment.highValue)
                {
                    retVal = false;
                    fieldDefFailReason_ = "Low value >= High value";
                }
                else if (inputSegment.stepSize <= 0)
                {
                    retVal = false;
                    fieldDefFailReason_ = "Step size <= 0";
                }
                else
                {
                    SegmentDef segment = inputSegment;
                    segment.numValues = GetNumValues(segment.lowValue, segment.highValue, segment.stepSize);

                    if (segment.numValues == 0)
                    {
                        retVal = false;
                        fieldDefFailReason_ = "Step size does not evenly divide the low-to-high range";
                    }
                    else if (i > 0 && ScaleUp(segment.lowValue) < ScaleUp(fd.segmentList[i - 1].highValue))
                    {
                        retVal = false;
                        fieldDefFailReason_ = "Segment ranges overlap";
                    }
                    else
                    {
                        fd.segmentList[i] = segment;

                        uint32_t additionalValues = segment.numValues;
                        if (i > 0 && ScaleUp(segment.lowValue) == ScaleUp(fd.segmentList[i - 1].highValue))
                        {
                            --additionalValues;
                        }

                        fd.numValues += additionalValues;
                    }
                }
            }

            if (retVal)
            {
                fd.lowValue = fd.segmentList[0].lowValue;
                fd.highValue = fd.segmentList[segmentCount - 1].highValue;
                fd.stepSize = 0;
                fd.numBits = std::log2(fd.numValues);

                if (numBitsSum_ + fd.numBits > MAX_BITS)
                {
                    retVal = false;
                    fieldDefFailReason_ = "Field overflows available bits";
                }
                else
                {
                    numBitsSum_ += fd.numBits;
                    fd.value = fd.lowValue;

                    fieldDefUserDefinedList_[fieldDefUserDefinedListIdx_] = fd;
                    ++fieldDefUserDefinedListIdx_;
                }
            }
        }

        return retVal;
    }

    template <size_t SEGMENT_COUNT>
    bool DefineField(const char *fieldName,
                     const std::array<SegmentDef, SEGMENT_COUNT> &segmentList)
    {
        return DefineField(fieldName, segmentList.data(), static_cast<uint8_t>(segmentList.size()));
    }

    void SetCanSetHdrType(bool val)
    {
        canSetHdrType_ = val;
    }

    // Read the default HdrType, or, read the value
    // which was set from Decode().
    HdrType GetHdrType()
    {
        return (HdrType)Get("HdrType");
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
                uint32_t fieldNumber = GetFieldNumber(fd, fd.value);

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
                fd.value = GetValueForFieldNumber(fd, fieldNumber);

                // shed
                val /= fd.numValues;
            }
        };

        // unpack header fields
        UnpackFields(val, static_cast<FieldDef *>(fieldDefHeaderList_.data()), fieldDefHeaderList_.size());

        // unpack application fields
        UnpackFields(val, static_cast<FieldDef *>(fieldDefUserDefinedList_.data()), fieldDefUserDefinedListIdx_);

        // validate only that this is Extended Telemetry
        retVal = GetHdrTelemetryType() == 0;

        return retVal;
    }


private:

    static constexpr int64_t SCALE_FACTOR = 10000;

    static int64_t ScaleUp(double value)
    {
        int64_t valueScaledUp = 0;

        if (value < 0)
        {
            valueScaledUp = (value * SCALE_FACTOR) - 0.5;
        }
        else if (value > 0)
        {
            valueScaledUp = (value * SCALE_FACTOR) + 0.5;
        }

        return valueScaledUp;
    }

    static bool FieldIsTooPrecise(double value)
    {
        int64_t valueScaledUp   = ScaleUp(value);
        double  valueScaledBack = static_cast<double>(valueScaledUp) / SCALE_FACTOR;

        double diff = fabs(valueScaledBack - value);

        return diff > 0.000000001;
    }

    static uint32_t GetNumValues(double lowValue, double highValue, double stepSize)
    {
        uint32_t retVal = 0;

        int64_t lowValueScaledUpAsInt  = ScaleUp(lowValue);
        int64_t highValueScaledUpAsInt = ScaleUp(highValue);
        int64_t stepSizeScaledUpAsInt  = ScaleUp(stepSize);

        double stepCount = ((double)(highValueScaledUpAsInt - lowValueScaledUpAsInt)) / stepSizeScaledUpAsInt;

        if (stepCount == (uint32_t)stepCount)
        {
            retVal = static_cast<uint32_t>(stepCount) + 1;
        }

        return retVal;
    }

    static uint32_t GetFieldNumber(const FieldDef &fd, double value)
    {
        uint32_t retVal = 0;

        if (fd.isSegmented == false)
        {
            retVal = (uint32_t)std::round((value - fd.lowValue) / fd.stepSize);
        }
        else
        {
            static constexpr double TIE_EPSILON = 1e-9;
            double bestDiff = INFINITY;
            double bestValue = fd.lowValue;
            uint32_t candidateFieldNumber = 0;

            for (uint8_t segmentIdx = 0; segmentIdx < fd.segmentCount; ++segmentIdx)
            {
                const SegmentDef &segment = fd.segmentList[segmentIdx];
                uint32_t offset = (segmentIdx > 0 &&
                                   ScaleUp(segment.lowValue) == ScaleUp(fd.segmentList[segmentIdx - 1].highValue)) ? 1 : 0;

                for (uint32_t i = offset; i < segment.numValues; ++i)
                {
                    double candidateValue = segment.lowValue + (segment.stepSize * i);
                    double diff = std::fabs(candidateValue - value);

                    if (diff < bestDiff ||
                        (std::fabs(diff - bestDiff) < TIE_EPSILON && candidateValue > bestValue))
                    {
                        bestDiff = diff;
                        bestValue = candidateValue;
                        retVal = candidateFieldNumber;
                    }

                    ++candidateFieldNumber;
                }
            }
        }

        return retVal;
    }

    static double GetValueForFieldNumber(const FieldDef &fd, uint32_t fieldNumber)
    {
        double retVal = 0;

        if (fd.isSegmented == false)
        {
            retVal = fd.lowValue + (fieldNumber * fd.stepSize);
        }
        else
        {
            uint32_t candidateFieldNumber = 0;

            for (uint8_t segmentIdx = 0; segmentIdx < fd.segmentCount; ++segmentIdx)
            {
                const SegmentDef &segment = fd.segmentList[segmentIdx];
                uint32_t offset = (segmentIdx > 0 &&
                                   ScaleUp(segment.lowValue) == ScaleUp(fd.segmentList[segmentIdx - 1].highValue)) ? 1 : 0;

                for (uint32_t i = offset; i < segment.numValues; ++i)
                {
                    if (candidateFieldNumber == fieldNumber)
                    {
                        return segment.lowValue + (segment.stepSize * i);
                    }

                    ++candidateFieldNumber;
                }
            }
        }

        return retVal;
    }

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
    const char *fieldDefFailReason_;

    double numBitsSum_;

    // never changes count, but values can change
    std::array<FieldDef, 4> fieldDefHeaderList_;

    bool canSetHdrType_;
};


