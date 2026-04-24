#pragma once

#include <array>
#include <cmath>
#include <cstdint>


template <typename WsprMessageTelemetryExtendedCommonType, uint8_t FIELD_COUNT = 7>
class WsprMessageTelemetryExtendedTrackerTelemetryBase
: public WsprMessageTelemetryExtendedCommonType
{
    using Base = WsprMessageTelemetryExtendedCommonType;

public:

    enum class GpsLockType : uint8_t
    {
        NO_LOCK = 0,
        TIME_LOCK = 1,
        LOCATION_LOCK = 2,
    };

    struct Location
    {
        uint8_t subLatIdx = 0;
        uint8_t subLngIdx = 0;
    };

    WsprMessageTelemetryExtendedTrackerTelemetryBase()
    {
        ResetEverything();
    }

    void ResetEverything()
    {
        Base::ResetEverything();

        static constexpr std::array<typename Base::SegmentDef, 2> temperatureSegments = {{
            { -80, 40, 5 },
            {  40, 64, 8 },
        }};
        static constexpr std::array<typename Base::SegmentDef, 3> voltageSegments = {{
            { 2.70, 3.10, 0.08 },
            { 3.10, 5.26, 0.06 },
            { 5.26, 6.06, 0.08 },
        }};

        Base::DefineField("Id13", 0, 19, 1);
        Base::DefineField("Temp", temperatureSegments);
        Base::DefineField("Voltage", voltageSegments);
        Base::DefineField("Window", 1, 6, 1);
        Base::DefineField("GpsLockType", 0, 2, 1);
        Base::DefineField("SubLat", 0, 33, 1);
        Base::DefineField("SubLng", 0, 33, 1);

        Base::SetCanSetHdrType(true);
        Base::Set("HdrType", static_cast<uint8_t>(Base::HdrType::TRACKER_TELEMETRY));
        Base::SetCanSetHdrType(false);
    }

    void SetId13Idx(uint8_t val)
    {
        Base::Set("Id13", val);
    }

    uint8_t GetId13Idx()
    {
        return static_cast<uint8_t>(Base::Get("Id13"));
    }

    void SetTempF(double val)
    {
        Base::Set("Temp", val);
    }

    double GetTempF()
    {
        return Base::Get("Temp");
    }

    void SetVoltageV(double val)
    {
        Base::Set("Voltage", val);
    }

    double GetVoltageV()
    {
        return Base::Get("Voltage");
    }

    void SetWindowSeqNo(uint8_t val)
    {
        Base::Set("Window", NormalizeWindowSeqNo(val));
    }

    uint8_t GetWindowSeqNo()
    {
        return static_cast<uint8_t>(Base::Get("Window"));
    }

    void SetGpsLockType(GpsLockType val)
    {
        Base::Set("GpsLockType", static_cast<uint8_t>(val));
    }

    GpsLockType GetGpsLockType()
    {
        return static_cast<GpsLockType>(static_cast<uint8_t>(Base::Get("GpsLockType")));
    }

    void SetSubLatIdx(uint8_t val)
    {
        Base::Set("SubLat", val);
    }

    uint8_t GetSubLatIdx()
    {
        return static_cast<uint8_t>(Base::Get("SubLat"));
    }

    void SetSubLngIdx(uint8_t val)
    {
        Base::Set("SubLng", val);
    }

    uint8_t GetSubLngIdx()
    {
        return static_cast<uint8_t>(Base::Get("SubLng"));
    }

    static uint8_t NormalizeWindowSeqNo(int value)
    {
        static constexpr uint8_t WINDOW_SEQ_NO_LOW = 1;
        static constexpr uint8_t WINDOW_SEQ_NO_HIGH = 6;

        uint8_t radix = WINDOW_SEQ_NO_HIGH - WINDOW_SEQ_NO_LOW + 1;

        return static_cast<uint8_t>(
            WINDOW_SEQ_NO_LOW
            + ((((value - WINDOW_SEQ_NO_LOW) % radix) + radix) % radix)
        );
    }

    static double GetSubLatCellHeightDeg()
    {
        return (180.0 / 127.0) / 34.0;
    }

    static double GetSubLngCellWidthDeg()
    {
        return (360.0 / 254.0) / 34.0;
    }

    static bool EncodeLocationToFieldValues(uint8_t gtLatitudeIdx,
                                            uint8_t gtLongitudeIdx,
                                            double latitude,
                                            double longitude,
                                            Location &location)
    {
        double south = 0.0;
        double west = 0.0;
        double north = 0.0;
        double east = 0.0;

        if (!GetGpsTelemetryLocationBounds(gtLatitudeIdx, gtLongitudeIdx, south, west, north, east))
        {
            return false;
        }

        if (!std::isfinite(latitude) || !std::isfinite(longitude))
        {
            return false;
        }

        if (latitude < south || latitude >= north || longitude < west || longitude >= east)
        {
            return false;
        }

        location.subLatIdx = static_cast<uint8_t>(std::floor((latitude - south) / GetSubLatCellHeightDeg()));
        location.subLngIdx = static_cast<uint8_t>(std::floor((longitude - west) / GetSubLngCellWidthDeg()));

        return true;
    }

    bool DecodeFieldValuesToLocation(uint8_t gtLatitudeIdx,
                                     uint8_t gtLongitudeIdx,
                                     double &latitude,
                                     double &longitude)
    {
        return DecodeFieldValuesToLocation(gtLatitudeIdx, gtLongitudeIdx, GetSubLatIdx(), GetSubLngIdx(), latitude, longitude);
    }

    static bool DecodeFieldValuesToLocation(uint8_t gtLatitudeIdx,
                                            uint8_t gtLongitudeIdx,
                                            uint8_t subLatIdx,
                                            uint8_t subLngIdx,
                                            double &latitude,
                                            double &longitude)
    {
        double south = 0.0;
        double west = 0.0;
        double north = 0.0;
        double east = 0.0;

        if (!GetLocationBoundsFromFieldValues(gtLatitudeIdx, gtLongitudeIdx, subLatIdx, subLngIdx, south, west, north, east))
        {
            return false;
        }

        latitude = (south + north) / 2.0;
        longitude = (west + east) / 2.0;

        return true;
    }

    static bool GetLocationBoundsFromFieldValues(uint8_t gtLatitudeIdx,
                                                 uint8_t gtLongitudeIdx,
                                                 uint8_t subLatIdx,
                                                 uint8_t subLngIdx,
                                                 double &south,
                                                 double &west,
                                                 double &north,
                                                 double &east)
    {
        double gtSouth = 0.0;
        double gtWest = 0.0;
        double gtNorth = 0.0;
        double gtEast = 0.0;

        if (!GetGpsTelemetryLocationBounds(gtLatitudeIdx, gtLongitudeIdx, gtSouth, gtWest, gtNorth, gtEast))
        {
            return false;
        }

        if (subLatIdx > 33 || subLngIdx > 33)
        {
            return false;
        }

        south = gtSouth + (subLatIdx * GetSubLatCellHeightDeg());
        west = gtWest + (subLngIdx * GetSubLngCellWidthDeg());
        north = south + GetSubLatCellHeightDeg();
        east = west + GetSubLngCellWidthDeg();

        return true;
    }

    bool Decode()
    {
        bool retVal = true;

        retVal &= Base::Decode();
        retVal &= Base::GetHdrType() == Base::HdrType::TRACKER_TELEMETRY;

        return retVal;
    }

private:

    static bool GetGpsTelemetryLocationBounds(uint8_t gtLatitudeIdx,
                                              uint8_t gtLongitudeIdx,
                                              double &south,
                                              double &west,
                                              double &north,
                                              double &east)
    {
        if (gtLatitudeIdx > 126 || gtLongitudeIdx > 253)
        {
            return false;
        }

        double gtLatCellHeightDeg = 180.0 / 127.0;
        double gtLngCellWidthDeg = 360.0 / 254.0;

        south = -90.0 + (gtLatitudeIdx * gtLatCellHeightDeg);
        west = -180.0 + (gtLongitudeIdx * gtLngCellWidthDeg);
        north = south + gtLatCellHeightDeg;
        east = west + gtLngCellWidthDeg;

        return true;
    }

    using Base::SetCanSetHdrType;
};
