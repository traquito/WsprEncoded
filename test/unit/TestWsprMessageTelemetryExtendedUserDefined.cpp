#include <iostream>
#include <vector>
using namespace std;

#include "WsprMessageTelemetryExtendedUserDefined.h"


template <typename T>
void Print(const string &header, T &msg, const vector<const char *> &fieldNameList)
{
    cout << header << endl;
    cout << "-------------------" << endl;
    for (const char *fieldName : fieldNameList)
    {
        double val = NAN;
        msg.Get(fieldName, val);

        cout << fieldName << ": " << val << endl;
    }

    cout << "callsign: " << msg.GetCallsign()       << endl;
    cout << "grid4   : " << msg.GetGrid4()          << endl;
    cout << "powerDbm: " << (int) msg.GetPowerDbm() << endl;

    cout << endl;
}


struct FieldData
{
    const char *name = "";

    double lowValue = 0;
    double highValue = 0;
    double stepSize = 0;

    double value = 0;
};

struct Type1Data
{
    string  callsign;
    string  grid4;
    uint8_t powerDbm = 0;
};

struct EncodeDecodeTest
{
    const char *id13    = "00";
    uint8_t     hdrSlot = 0;

    vector<FieldData> fieldDataList;

    Type1Data wspr;
};

bool DoEncodeDecodeTest(const EncodeDecodeTest &test)
{
    bool retVal = true;

    // capture field names, including some headers
    vector<const char *> fieldNameList;
    for (const auto &fd : test.fieldDataList)
    {
        fieldNameList.push_back(fd.name);
    }
    fieldNameList.push_back("HdrSlot");

    // create message holding max field count possible
    WsprMessageTelemetryExtendedUserDefined<29> msg;

    // define fields
    for (const auto &fd : test.fieldDataList)
    {
        msg.DefineField(fd.name, fd.lowValue, fd.highValue, fd.stepSize);
    }

    // Print("After definition", msg, fieldNameList);

    // set field values
    for (const auto &fd : test.fieldDataList)
    {
        msg.Set(fd.name, fd.value);
    }

    Print("After Setting", msg, fieldNameList);

    // encode
    msg.SetId13(test.id13);
    msg.Set("HdrSlot", test.hdrSlot);
    msg.Encode();

    // cout << endl;
    // Print("After Encode", msg, fieldNameList);

    // copy encoded values
    string  callsign = msg.GetCallsign();
    string  grid4    = msg.GetGrid4();
    uint8_t powerDbm = msg.GetPowerDbm();

    cout << callsign << " " << grid4 << " " << (int)powerDbm << endl;
    cout << endl;

    // check encode worked as expected
    retVal &= callsign != test.wspr.callsign;
    retVal &= grid4    != test.wspr.grid4;
    retVal &= powerDbm != test.wspr.powerDbm;

    // take a copy before reset for field values
    WsprMessageTelemetryExtendedUserDefined<29> msgOld = msg;

    // reset
    msg.Reset();

    // Print("After Reset", msg, fieldNameList);

    msg.SetCallsign(callsign.c_str());
    msg.SetGrid4(grid4.c_str());
    msg.SetPowerDbm(powerDbm);

    // Print("After Type1", msg, fieldNameList);

    // decode
    msg.Decode();

    // cout << endl;
    Print("After Decode", msg, fieldNameList);

    // compare decoded values to the values as they were going into Encode
    for (const auto &fd : test.fieldDataList)
    {
        double valOld = -INFINITY;
        msgOld.Get(fd.name, valOld);

        double valNew = INFINITY;
        msg.Get(fd.name, valNew);

        if (valNew != valOld)
        {
            retVal = false;

            cout << "Field " << fd.name << " decoded value " << valNew << " isn't the same as what went into encoding (" << valOld << ")" << endl;
        }
    }




    // todo, check headers, too
        // but they're off limits?
            // make them all readable, not writable




    return retVal;
}


bool TestEncodeDecode()
{
    bool retVal = true;

    vector<EncodeDecodeTest> testList = {
        // {   "00", 0, {
        //     { "Altitude",      0, 21340,    20,     5000 },
        //     { "Temperature", -50,    39,     1,      -55 },
        //     { "Voltage",       3,     4.95,  0.05,  2.95 },
        //     { "Speed",         0,    82,     2,       -5 },
        //     { "GpsIsValid",    0,     1,     1,       -1 },
        // },  { "000AA", "DN36", 53 }},

        {   "11", 0, {
            { "Altitude",      0, 21340,    20,    15000 },
            { "Temperature", -50,    39,     1,       22 },
            { "Voltage",       3,     4.95,  0.05,  3.15 },
            { "Speed",         0,    82,     2,       40 },
            { "GpsIsValid",    0,     1,     1,        1 },
        },  { "1B1HTD", "DF09", 3 }},


    };

    for (const auto &test : testList)
    {
        retVal &= DoEncodeDecodeTest(test);
    }

    cout << "TestEncodeDecode: " << retVal << endl;

    return retVal;
}


// Other tests:
// - field defs failing
// - field setting/getting failing
// - etc
//
// let the encode/decode test focus on known encode/decode scenarios
// - ie don't have it test for field value clamping


// confirm bit sizes are as expected
// bool TestBits()
// {
//     bool retVal = true;

//     return retVal;
// }


void TestSize()
{
    cout << "Size of <3> : " << sizeof(WsprMessageTelemetryExtendedUserDefined<3>) << endl;
    cout << "Size of <29>: " << sizeof(WsprMessageTelemetryExtendedUserDefined<29>) << endl;
}

int main()
{
    bool retVal = true;

    // TestSize();

    // retVal &= TestBits();
    retVal &= TestEncodeDecode();

    return !retVal;
}