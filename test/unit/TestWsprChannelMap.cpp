#include <iostream>
#include <array>
#include <vector>
using namespace std;

#include "TestUtl.h"
using namespace TestUtl;

#include "WsprChannelMap.h"


bool TestGetChannelDetails()
{
    struct Arg
    {
        const char *band;
        uint16_t channel;
    };

    // subset of fields I care to check
    struct Result
    {
        char     id1;
        char     id3;
        uint8_t  min;
        uint32_t freq;

        // make compare work
        auto operator<=>(const Result &) const = default;
    };

    vector<InputTest<Arg, Result>> testList = {
        { { "20m",   0 }, { '0', '0', 8, 14097020, }, "20m,   0", },
        { { "20m",   1 }, { '0', '0', 0, 14097020, }, "20m,   1", },
        { { "20m", 226 }, { '1', '1', 0, 14097060, }, "20m, 226", },
        { { "20m", 452 }, { 'Q', '2', 2, 14097140, }, "20m, 252", },
        { { "20m", 452 }, { 'Q', '2', 2, 14097140, }, "20m, 252", },
        { { "21m",   1 }, { '0', '0', 0, 14097020, }, "21m,   1", },
        { { "20m", 600 }, { '0', '0', 8, 14097020, }, "20m, 600", },
    };

    bool retVal = true;
    for (const auto &test : testList)
    {
        WsprChannelMap::ChannelDetails cd = WsprChannelMap::GetChannelDetails(test.input.band, test.input.channel);

        Result r = { cd.id1, cd.id3, cd.min, cd.freq, };

        if (test.expectedRetVal != r)
        {
            retVal = false;

            cout << "ERR: Got diff val than expected (" << test.comment << ")" << endl;
        }
    }

    cout << "TestGetChannelDetails: " << retVal << endl;

    return retVal;
}

bool TestGetDefaultChannelIfNotValid()
{
    vector<InputTest<uint16_t, uint16_t>> testList = {
        {   0,   0, "valid",   },
        { 187, 187, "valid",   },
        { 599, 599, "valid",   },
        { 600,   0, "invalid", },
    };

    bool retVal = true;
    for (const auto &test : testList)
    {
        retVal &= CheckErr(test, WsprChannelMap::GetDefaultChannelIfNotValid(test.input));
    }

    cout << "TestGetDefaultChannelIfNotValid: " << retVal << endl;

    return retVal;
}

bool TestGetId13IndexForChannel()
{
    vector<InputTest<uint16_t, uint8_t>> testList = {
        {   0, 0, "channel 0",   },
        {   3, 3, "channel 3",   },
        {  20, 0, "channel 20",  },
        { 226, 6, "channel 226", },
        { 483, 3, "channel 483", },
        { 599, 19, "channel 599", },
    };

    bool retVal = true;
    for (const auto &test : testList)
    {
        retVal &= CheckErr(test, WsprChannelMap::GetId13IndexForChannel("20m", test.input));
    }

    cout << "TestGetId13IndexForChannel: " << retVal << endl;

    return retVal;
}

bool TestGetChannelDetailsFromId13AndIdx()
{
    struct Arg
    {
        const char *id13;
        uint8_t id13Idx;
    };

    vector<InputTest<Arg, uint16_t>> testList = {
        { { "00", 0 }, 0,   "00, idx 0",  },
        { { "00", 1 }, 1,   "00, idx 1",  },
        { { "11", 6 }, 226, "11, idx 6",  },
        { { "Q4", 3 }, 483, "Q4, idx 3",  },
        { { "Q9", 19 }, 599, "Q9, idx 19", },
    };

    bool retVal = true;
    for (const auto &test : testList)
    {
        auto cd = WsprChannelMap::GetChannelDetailsFromId13AndIdx("20m", test.input.id13, test.input.id13Idx);
        retVal &= CheckErr(test, cd.channel);
    }

    cout << "TestGetChannelDetailsFromId13AndIdx: " << retVal << endl;

    return retVal;
}

bool TestSweepEveryBandAndChannelForward()
{
    bool retVal = true;

    for (const auto &bandData : Wspr::GetBandDataList())
    {
        std::array<bool, 600> seenChannelList = {};

        for (uint16_t channel = 0; channel <= 599; ++channel)
        {
            WsprChannelMap::ChannelDetails cd = WsprChannelMap::GetChannelDetails(bandData.band, channel);
            uint8_t id13Idx = WsprChannelMap::GetId13IndexForChannel(bandData.band, channel);
            WsprChannelMap::ChannelDetails cdRoundTrip = WsprChannelMap::GetChannelDetailsFromId13AndIdx(bandData.band, cd.id13, id13Idx);

            bool ok =
                cd.channel == channel &&
                cdRoundTrip.channel == channel &&
                id13Idx <= 19 &&
                cd.id13[0] == cd.id1 &&
                cd.id13[1] == cd.id3 &&
                cd.id13[2] == '\0';

            if (!ok)
            {
                retVal = false;

                cout << "ERR: Forward sweep mismatch on band " << bandData.band
                     << ", channel " << channel
                     << " (id13=" << cd.id13
                     << ", idx=" << (int)id13Idx
                     << ", roundTrip=" << cdRoundTrip.channel << ")"
                     << endl;
            }

            if (seenChannelList[channel])
            {
                retVal = false;

                cout << "ERR: Duplicate channel encountered on band " << bandData.band
                     << ", channel " << channel
                     << endl;
            }

            seenChannelList[channel] = true;
        }

        for (uint16_t channel = 0; channel <= 599; ++channel)
        {
            if (!seenChannelList[channel])
            {
                retVal = false;

                cout << "ERR: Missing channel on band " << bandData.band
                     << ", channel " << channel
                     << endl;
            }
        }
    }

    cout << "TestSweepEveryBandAndChannelForward: " << retVal << endl;

    return retVal;
}

bool TestSweepEveryBandAndId13IdxReverse()
{
    bool retVal = true;

    const std::array<char, 3> id1List = { '0', '1', 'Q' };
    const std::array<char, 10> id3List = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

    for (const auto &bandData : Wspr::GetBandDataList())
    {
        std::array<bool, 600> seenChannelList = {};
        uint16_t seenCount = 0;

        for (char id1 : id1List)
        {
            for (char id3 : id3List)
            {
                char id13[3] = { id1, id3, '\0' };

                for (uint8_t id13Idx = 0; id13Idx <= 19; ++id13Idx)
                {
                    WsprChannelMap::ChannelDetails cd = WsprChannelMap::GetChannelDetailsFromId13AndIdx(bandData.band, id13, id13Idx);
                    uint8_t id13IdxRoundTrip = WsprChannelMap::GetId13IndexForChannel(bandData.band, cd.channel);
                    WsprChannelMap::ChannelDetails cdRoundTrip = WsprChannelMap::GetChannelDetails(bandData.band, cd.channel);

                    bool ok =
                        cd.channel <= 599 &&
                        id13IdxRoundTrip == id13Idx &&
                        string(cd.id13) == string(id13) &&
                        string(cdRoundTrip.id13) == string(id13) &&
                        cdRoundTrip.channel == cd.channel;

                    if (!ok)
                    {
                        retVal = false;

                        cout << "ERR: Reverse sweep mismatch on band " << bandData.band
                             << ", id13 " << id13
                             << ", idx " << (int)id13Idx
                             << " (channel=" << cd.channel
                             << ", roundTripIdx=" << (int)id13IdxRoundTrip
                             << ", roundTripId13=" << cdRoundTrip.id13 << ")"
                             << endl;
                    }

                    if (seenChannelList[cd.channel])
                    {
                        retVal = false;

                        cout << "ERR: Duplicate reverse-mapped channel on band " << bandData.band
                             << ", channel " << cd.channel
                             << endl;
                    }
                    else
                    {
                        seenChannelList[cd.channel] = true;
                        ++seenCount;
                    }
                }
            }
        }

        if (seenCount != 600)
        {
            retVal = false;

            cout << "ERR: Reverse sweep count mismatch on band " << bandData.band
                 << ", saw " << seenCount
                 << " channels"
                 << endl;
        }

        for (uint16_t channel = 0; channel <= 599; ++channel)
        {
            if (!seenChannelList[channel])
            {
                retVal = false;

                cout << "ERR: Reverse sweep missing channel on band " << bandData.band
                     << ", channel " << channel
                     << endl;
            }
        }
    }

    cout << "TestSweepEveryBandAndId13IdxReverse: " << retVal << endl;

    return retVal;
}

int main()
{
    bool retVal = true;

    retVal &= TestGetDefaultChannelIfNotValid();
    retVal &= TestGetChannelDetails();
    retVal &= TestGetId13IndexForChannel();
    retVal &= TestGetChannelDetailsFromId13AndIdx();
    retVal &= TestSweepEveryBandAndChannelForward();
    retVal &= TestSweepEveryBandAndId13IdxReverse();

    return !retVal;
}
