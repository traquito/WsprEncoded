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
        string      comment;
    };

    vector<Grid4FromLatLngTest> testList = {
        {  40.738059,  -74.036227, "FN20", "New Jersey example",        },
        { -26.301960,  -66.709667, "FG63", "Argentina example",         },
        { -31.951585,  115.824861, "OF78", "Australia example",         },
        {  10.813707,  106.609537, "OK30", "Vietnam example",           },
        { -90.000000, -180.000000, "AA00", "southwest world boundary",  },
        {  90.000000,  180.000000, "RR99", "northeast world boundary",  },
        {   0.000000,  181.000000, "AJ00", "longitude wraps east",      },
        {   0.000000, -181.000000, "RJ90", "longitude wraps west",      },
    };

    bool retVal = true;

    for (const auto &test : testList)
    {
        char grid4[5] = { 0 };
        bool ok = WsprMessageRegularType1::Grid4FromLatLng(test.latitude, test.longitude, grid4, sizeof(grid4));
        if (!ok || strcmp(grid4, test.expectedGrid4) != 0)
        {
            retVal = false;
            cout << "ERR: Got " << grid4 << " with ok=" << ok
                 << ", but expected " << test.expectedGrid4
                 << " (" << test.comment << ")" << endl;
        }
    }

    char grid4[5] = { 0 };
    retVal &= CheckErr(InputTest<char *, bool>{ nullptr, false, "nullptr output buffer" },
                       WsprMessageRegularType1::Grid4FromLatLng(0.0, 0.0, nullptr, sizeof(grid4)));
    retVal &= CheckErr(InputTest<size_t, bool>{ 4, false, "output buffer too small" },
                       WsprMessageRegularType1::Grid4FromLatLng(0.0, 0.0, grid4, 4));

    cout << "Grid4FromLatLng test: " << retVal << endl;

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
