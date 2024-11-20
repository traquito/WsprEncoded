#include "WsprEncoded.h"

using std::string;
using std::to_string;


/////////////////////////////////////////////////////////////////////
// Testing Channel Detail Lookup
/////////////////////////////////////////////////////////////////////

void ReportChannelDetails(string band, uint16_t channel)
{
    // Report the requested band and channel
    Serial.print("[Channel details for band ");
    Serial.print(band.c_str());
    Serial.print(", channel ");
    Serial.print(channel);
    Serial.print("]");
    Serial.println();

    // Look up channel details by band and channel
    WSPR::ChannelDetails cd = WSPR::GetChannelDetails(band, channel);

    // Report the channel details for the given band and channel    
    Serial.print("id1 : "); Serial.println(cd.id1);
    Serial.print("id3 : "); Serial.println(cd.id3);
    Serial.print("id13: "); Serial.println(cd.id13.c_str());
    Serial.print("min : "); Serial.println(cd.min);
    Serial.print("freq: "); Serial.println(cd.freq);
    Serial.println();
}

void TestLookupChannelDetails()
{
    string   band    = "20m";
    uint16_t channel = 368;

    ReportChannelDetails(band, channel);
    Serial.println();
}


/////////////////////////////////////////////////////////////////////
// Testing Message Encoding
/////////////////////////////////////////////////////////////////////

void ReportEncodeU4B(string  id13,
                     string  grid56,
                     int32_t altM,
                     int8_t  tempC,
                     double  voltage,
                     uint8_t speedKnots,
                     bool    gpsValid)
{
    // Create the message encoder
    WSPRMessageU4B msgU4b;

    // Set the telemetry fields
    msgU4b.SetId13(id13);
    msgU4b.SetGrid56(grid56);
    msgU4b.SetAltM(altM);
    msgU4b.SetTempC(tempC);
    msgU4b.SetVoltage(voltage);
    msgU4b.SetSpeedKnots(speedKnots);
    msgU4b.SetGpsValid(gpsValid);

    // Report the parameters passed, and if they got automatically clamped
    Serial.println("Encoded WSPR U4B Type1 Message for:");
    Serial.print("id13      : input as  : "); Serial.println(id13.c_str());
    Serial.print("          : clamped to: "); Serial.println(msgU4b.GetId13().c_str());
    Serial.print("grid56    : input as  : "); Serial.println(grid56.c_str());
    Serial.print("          : clamped to: "); Serial.println(msgU4b.GetGrid56().c_str());
    Serial.print("altM      : input as  : "); Serial.println(altM);
    Serial.print("          : clamped to: "); Serial.println(msgU4b.GetAltM());
    Serial.print("tempC     : input as  : "); Serial.println(tempC);
    Serial.print("          : clamped to: "); Serial.println(msgU4b.GetTempC());
    Serial.print("voltage   : input as  : "); Serial.println(voltage);
    Serial.print("          : clamped to: "); Serial.println(msgU4b.GetVoltage());
    Serial.print("speedKnots: input as  : "); Serial.println(speedKnots);
    Serial.print("          : clamped to: "); Serial.println(msgU4b.GetSpeedKnots());
    Serial.print("gpsValid  : input as  : "); Serial.println(gpsValid);
    Serial.print("          : clamped to: "); Serial.println(msgU4b.GetGpsValid());
    Serial.println();

    // Extract the WSPR Type1 Message fields from the encoder
    string  callsign = msgU4b.GetCallsign();
    string  grid4    = msgU4b.GetGrid();
    uint8_t pwrDbm   = msgU4b.GetPowerDbm();

    // Report what the Type1 Message fields are
    Serial.print("callsign: "); Serial.println(callsign.c_str());
    Serial.print("grid4   : "); Serial.println(grid4.c_str());
    Serial.print("pwrDbm  : "); Serial.println(pwrDbm);
    Serial.println();

    // Give a URL to check decoding at
    string url = "";
    url += "https://traquito.github.io/pro/decode/";
    url += "?decode=";
    url += callsign;
    url += "%20";
    url += grid4;
    url += "%20";
    url += to_string(pwrDbm);
    url += "&encode=";

    Serial.print("Check decoding at: "); Serial.println(url.c_str());
    Serial.println();
}

void TestEncodeU4B_NonClampedValues()
{
    string  id13       = "Q5";
    string  grid56     = "JM";
    int32_t altM       = 5'120;
    int8_t  tempC      = -5;
    double  voltage    = 3.25;
    uint8_t speedKnots = 25;
    bool    gpsValid   = true;

    Serial.println("[Testing Non-Clamped Encoded Values]");
    ReportEncodeU4B(id13, grid56, altM, tempC, voltage, speedKnots, gpsValid);
}

void TestEncodeU4B_ClampedValues()
{
    string  id13       = "Q5";
    string  grid56     = "JM";
    int32_t altM       = 25'000;
    int8_t  tempC      = 45;
    double  voltage    = 5.6;
    uint8_t speedKnots = 96;
    bool    gpsValid   = true;

    Serial.println("[Testing Clamped Encoded Values]");
    ReportEncodeU4B(id13, grid56, altM, tempC, voltage, speedKnots, gpsValid);
}

void TestEncodeU4B()
{
    TestEncodeU4B_NonClampedValues();
    Serial.println();
    
    TestEncodeU4B_ClampedValues();
    Serial.println();
}


/////////////////////////////////////////////////////////////////////
// Setup and Loop logic
/////////////////////////////////////////////////////////////////////

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    Serial.println("--------------");
    Serial.println("Start of tests");
    Serial.println("--------------");
    Serial.println();

    TestLookupChannelDetails();
    TestEncodeU4B();

    Serial.println();

    delay(5'000);
}





