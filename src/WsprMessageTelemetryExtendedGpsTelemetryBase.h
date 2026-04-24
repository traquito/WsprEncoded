#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>


template <typename WsprMessageTelemetryExtendedCommonType, uint8_t FIELD_COUNT = 4>
class WsprMessageTelemetryExtendedGpsTelemetryBase
: public WsprMessageTelemetryExtendedCommonType
{
    using Base = WsprMessageTelemetryExtendedCommonType;

public:

    struct Location
    {
        uint8_t latitudeIdx = 0;
        uint8_t longitudeIdx = 0;
    };

    WsprMessageTelemetryExtendedGpsTelemetryBase()
    {
        ResetEverything();
    }

    void ResetEverything()
    {
        Base::ResetEverything();

        static constexpr std::array<typename Base::SegmentDef, 8> altitudeSegments = {{
            {     0,  6000,  75 },
            {  6000, 10000, 200 },
            { 10000, 27000, 250 },
            { 27000, 30000, 200 },
            { 30000, 36000, 150 },
            { 36000, 45000,  75 },
            { 45000, 54000, 150 },
            { 54000, 65000, 500 },
        }};
        static constexpr std::array<typename Base::SegmentDef, 4> speedSegments = {{
            {   0, 100,  5 },
            { 100, 170,  7 },
            { 170, 250, 10 },
            { 250, 310, 12 },
        }};

        Base::DefineField("Latitude",  0, 126, 1);
        Base::DefineField("Longitude", 0, 253, 1);
        Base::DefineField("Altitude", altitudeSegments);
        Base::DefineField("Speed", speedSegments);

        Base::SetCanSetHdrType(true);
        Base::Set("HdrType", static_cast<uint8_t>(Base::HdrType::GPS_TELEMETRY));
        Base::SetCanSetHdrType(false);
    }

    void SetLatitudeIdx(uint8_t val)
    {
        Base::Set("Latitude", val);
    }

    uint8_t GetLatitudeIdx()
    {
        return static_cast<uint8_t>(Base::Get("Latitude"));
    }

    void SetLongitudeIdx(uint8_t val)
    {
        Base::Set("Longitude", val);
    }

    uint8_t GetLongitudeIdx()
    {
        return static_cast<uint8_t>(Base::Get("Longitude"));
    }

    void SetAltitudeFt(double val)
    {
        Base::Set("Altitude", val);
    }

    double GetAltitudeFt()
    {
        return Base::Get("Altitude");
    }

    void SetSpeedMPH(double val)
    {
        Base::Set("Speed", val);
    }

    double GetSpeedMPH()
    {
        return Base::Get("Speed");
    }

    static bool EncodeLocationToFieldValues(double latitude, double longitude, Location &location)
    {
        if (!std::isfinite(latitude) || !std::isfinite(longitude))
        {
            return false;
        }

        if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0)
        {
            return false;
        }

        double clampedLatitude = std::max(-90.0, std::min(90.0 - std::numeric_limits<double>::epsilon(), latitude));
        double clampedLongitude = std::max(-180.0, std::min(180.0 - std::numeric_limits<double>::epsilon(), longitude));

        location.latitudeIdx = static_cast<uint8_t>(std::floor((clampedLatitude + 90.0) / GetLatitudeCellHeightDeg()));
        location.longitudeIdx = static_cast<uint8_t>(std::floor((clampedLongitude + 180.0) / GetLongitudeCellWidthDeg()));

        return true;
    }

    bool DecodeFieldValuesToLocation(double &latitude, double &longitude)
    {
        return DecodeFieldValuesToLocation(GetLatitudeIdx(), GetLongitudeIdx(), latitude, longitude);
    }

    static bool DecodeFieldValuesToLocation(uint8_t latitudeIdx, uint8_t longitudeIdx, double &latitude, double &longitude)
    {
        double south = 0.0;
        double west = 0.0;
        double north = 0.0;
        double east = 0.0;

        if (!GetLocationBoundsFromFieldValues(latitudeIdx, longitudeIdx, south, west, north, east))
        {
            return false;
        }

        latitude = (south + north) / 2.0;
        longitude = (west + east) / 2.0;

        return true;
    }

    static bool GetLocationBoundsFromFieldValues(uint8_t latitudeIdx,
                                                 uint8_t longitudeIdx,
                                                 double &south,
                                                 double &west,
                                                 double &north,
                                                 double &east)
    {
        if (latitudeIdx > 126 || longitudeIdx > 253)
        {
            return false;
        }

        south = -90.0 + (latitudeIdx * GetLatitudeCellHeightDeg());
        west = -180.0 + (longitudeIdx * GetLongitudeCellWidthDeg());
        north = south + GetLatitudeCellHeightDeg();
        east = west + GetLongitudeCellWidthDeg();

        return true;
    }

    static double GetLatitudeCellHeightDeg()
    {
        return 180.0 / 127.0;
    }

    static double GetLongitudeCellWidthDeg()
    {
        return 360.0 / 254.0;
    }

    bool Decode()
    {
        bool retVal = true;

        retVal &= Base::Decode();
        retVal &= Base::GetHdrType() == Base::HdrType::GPS_TELEMETRY;

        return retVal;
    }

private:

    using Base::SetCanSetHdrType;
};
