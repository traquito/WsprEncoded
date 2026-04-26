#include <iostream>
#include <cstring>
#include <string>
#include <vector>
using namespace std;


#include "TestUtl.h"
using namespace TestUtl;

#include "WsprMessageRegularType1.h"


bool TestCall()
{
    vector<InputTest<const char *, bool>> testList = {
        { nullptr,      false,  "nullptr",                                              },
        { "",           false,  "empty string",                                         },
        { "K1J",        false,  "too short",                                            },
        { "K1JT",       true,   "valid",                                                },
        { "K1JT2",      true,   "invalid format but right len (we don't check format)", },
        { "K1JT22",     true,   "invalid format but right len (we don't check format)", },
        { "K1JT2 ",     false,  "invalid format -- padded",                             },
        { " K1JT2",     false,  "invalid format -- padded",                             },
        { " K1JT ",     false,  "invalid format -- padded",                             },
        { "K1JKT222",   false,  "too long",                                             },
    };

    bool retVal = true;

    for (const auto &test : testList)
    {
        WsprMessageRegularType1 msg;
        retVal &= CheckErr(test, msg.SetCallsign(test.input));
    }

    cout << "Call test: " << retVal << endl;

    return retVal;
}

bool TestGrid()
{
    vector<InputTest<const char *, bool>> testList = {
        { nullptr,     false, "nullptr",        },
        { "",          false, "empty string",   },
        { "F",         false, "too short",      },
        { " F",        false, "padded",         },
        { "F ",        false, "padded",         },
        { " F ",       false, "padded",         },
        { "FN",        false, "too short",      },
        { " FN",       false, "padded",         },
        { "FN ",       false, "padded",         },
        { " FN ",      false, "padded",         },
        { "FNA",       false, "invalid format", },
        { "FN2",       false, "too short",      },
        { "FN20",      true,  "valid",          },
        { "FN2A",      false, "invalid format", },
        { "FNA0",      false, "invalid format", },
        { "FS20",      false, "invalid format", },
        { "SN20",      false, "invalid format", },
        { "FN201",     false, "too long",       },
    };

    bool retVal = true;

    for (const auto &test : testList)
    {
        WsprMessageRegularType1 msg;
        retVal &= CheckErr(test, msg.SetGrid4(test.input));
    }

    cout << "Grid test: " << retVal << endl;

    return retVal;
}

bool TestPower()
{
    vector<InputTest<uint8_t, bool>> testList = {
        { 0,  true,  "valid",   },
        { 1,  false, "invalid", },
        { 60, true,  "valid",   },
        { 61, false, "invalid", },
    };

    bool retVal = true;

    for (const auto &test : testList)
    {
        WsprMessageRegularType1 msg;
        retVal &= CheckErr(test, msg.SetPowerDbm(test.input));
    }

    cout << "PowerDbm test: " << retVal << endl;

    return retVal;
}

bool TestGrid4FromLatLng()
{
    struct Grid4FromLatLngTest
    {
        double      latitude;
        double      longitude;
        const char *expectedGrid4;
        const char *expectedGrid6;
        string      comment;
    };

    vector<Grid4FromLatLngTest> testList = {
        {  40.738059,  -74.036227, "FN20", "FN20XR", "New Jersey example",        },
        { -26.301960,  -66.709667, "FG63", "FG63PQ", "Argentina example",         },
        { -31.951585,  115.824861, "OF78", "OF78VB", "Australia example",         },
        {  10.813707,  106.609537, "OK30", "OK30HT", "Vietnam example",           },
        { -90.000000, -180.000000, "AA00", "AA00AA", "southwest world boundary",  },
        {  90.000000,  180.000000, "RR99", "RR99XX", "northeast world boundary",  },
        {   0.000000,  181.000000, "AJ00", "AJ00LA", "longitude wraps east",      },
        {   0.000000, -181.000000, "RJ90", "RJ90LA", "longitude wraps west",      },
    };

    bool retVal = true;

    for (const auto &test : testList)
    {
        char grid4[5] = { 0 };
        char grid6[7] = { 0 };
        bool ok = WsprMessageRegularType1::Grid4FromLatLng(test.latitude, test.longitude, grid4, sizeof(grid4));
        if (!ok || strcmp(grid4, test.expectedGrid4) != 0)
        {
            retVal = false;
            cout << "ERR: Got " << grid4 << " with ok=" << ok
                 << ", but expected " << test.expectedGrid4
                 << " (" << test.comment << ")" << endl;
        }
        ok = WsprMessageRegularType1::Grid6FromLatLng(test.latitude, test.longitude, grid6, sizeof(grid6));
        if (!ok || strcmp(grid6, test.expectedGrid6) != 0)
        {
            retVal = false;
            cout << "ERR: Got " << grid6 << " with ok=" << ok
                 << ", but expected " << test.expectedGrid6
                 << " (" << test.comment << ")" << endl;
        }
    }

    char grid4[5] = { 0 };
    char grid6[7] = { 0 };
    retVal &= CheckErr(InputTest<char *, bool>{ nullptr, false, "nullptr output buffer" },
                       WsprMessageRegularType1::Grid4FromLatLng(0.0, 0.0, nullptr, sizeof(grid4)));
    retVal &= CheckErr(InputTest<size_t, bool>{ 4, false, "output buffer too small" },
                       WsprMessageRegularType1::Grid4FromLatLng(0.0, 0.0, grid4, 4));
    retVal &= CheckErr(InputTest<char *, bool>{ nullptr, false, "nullptr grid6 output buffer" },
                       WsprMessageRegularType1::Grid6FromLatLng(0.0, 0.0, nullptr, sizeof(grid6)));
    retVal &= CheckErr(InputTest<size_t, bool>{ 6, false, "grid6 output buffer too small" },
                       WsprMessageRegularType1::Grid6FromLatLng(0.0, 0.0, grid6, 6));

    cout << "Grid4/Grid6 FromLatLng test: " << retVal << endl;

    return retVal;
}

int main()
{
    bool retVal = true;

    retVal &= TestCall();
    retVal &= TestGrid();
    retVal &= TestPower();
    retVal &= TestGrid4FromLatLng();

    return !retVal;
}
