#include <iostream>
#include <vector>
using namespace std;

#include "WsprMessageRegularType1.h"


#include <iterator>
#include <string>
#include "../WSPRMessage.h"

// must be in ascii order
static string charset = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
char CalcNextChar(char c)
{
    char retVal = c;

    auto it = std::find(charset.begin(), charset.end(), c);

    if (it != charset.end())
    {
        int idx = distance(charset.begin(), it);

        int idxNew = (idx + 1) % charset.size();

        retVal = charset[idxNew];
    }
    else
    {
        retVal = charset[0];
    }

    return retVal;
}

char nextCharLookup[256];
void PreCalculateNext()
{
    for (int i = 0; i < 256; ++i)
    {
        nextCharLookup[i] = CalcNextChar((char)i);
    }
}

inline char GetNextChar(char c)
{
    static bool once = false;

    if (once == false)
    {
        once = true;

        PreCalculateNext();
    }

    return nextCharLookup[c];
}

void GetNextString(string &str)
{
    for (int i = str.length() - 1; i >= 0; --i)
    {
        char c    = str[i];
        char cNew = GetNextChar(c);

        str[i] = cNew;

        if (cNew >= c)
        {
            break;
        }
    }
}

void CompareToEndOfUniverse()
{
    string callOrig = "";

    for (int callLen = 0; callLen <= 7; ++callLen)
    {
        string gridOrig = "";

        for (int gridLen = 0; gridLen <= 5; ++gridLen)
        {
            for (int powerDbm = 0; powerDbm <= 255; ++powerDbm)
            {
                string call = callOrig;

                do
                {
                    string grid = gridOrig;

                    do
                    {
                        // will experience:
                        // every call length and permutation
                        // every grid length and permutation
                        // every power

                        static uint64_t count = 0;
                        ++count;
                        if ((count % 10'000'000) == 0)
                        {
                            cout << "call(" << call << "), grid(" << grid << "), powerDbm(" << powerDbm << ")" << endl;
                        }

                        WsprMessageRegularType1 msg;

                        msg.SetCallsign(call.c_str());
                        msg.SetGrid(grid.c_str());
                        msg.SetPowerDbm((uint8_t)powerDbm);



                        GetNextString(grid);
                    } while (grid != gridOrig);

                    GetNextString(call);
                } while (call != callOrig);
            }

            cout << "New grid len" << endl;

            gridOrig += " ";
        }


        callOrig += " ";
    }
}



template <typename T>
struct InputTest
{
    T      input;
    bool   expectedRetVal;
    string comment;
};

template <typename T>
bool CheckErr(const InputTest<T> &test, bool testRetVal)
{
    bool retVal = true;

    if (testRetVal != test.expectedRetVal)
    {
        retVal = false;

        cout << "ERR: Got " << testRetVal << ", but expected " << test.expectedRetVal << " (" << test.comment << ")" << endl;
    }

    return retVal;
}

bool TestCall()
{
    vector<InputTest<string>> testList = {
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
        retVal &= CheckErr(test, msg.SetCallsign(test.input.c_str()));
    }

    for (const auto &test : testList)
    {
        WSPRMessage msg;
        retVal &= CheckErr(test, msg.SetCallsign(test.input.c_str()));
    }

    cout << "Call test: " << retVal << endl;

    return retVal;
}

bool TestGrid()
{
    vector<InputTest<string>> testList = {
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
        retVal &= CheckErr(test, msg.SetGrid(test.input.c_str()));
    }
    for (const auto &test : testList)
    {
        WSPRMessage msg;
        retVal &= CheckErr(test, msg.SetGrid(test.input.c_str()));
    }

    cout << "Grid test: " << retVal << endl;

    return retVal;
}

bool TestPower()
{
    vector<InputTest<uint8_t>> testList = {
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
    for (const auto &test : testList)
    {
        WSPRMessage msg;
        retVal &= CheckErr(test, msg.SetPowerDbm(test.input));
    }

    cout << "PowerDbm test: " << retVal << endl;

    return retVal;
}

int main(int argc, char *argv[])
{
    bool ok = true;

    ok &= TestCall();
    ok &= TestGrid();
    ok &= TestPower();

    return ok == false;
}