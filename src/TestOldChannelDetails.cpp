#include <iostream>
using namespace std;

#include "WSPR.h"


int main(int argc, char *argv[])
{
    cout << "Channel   0: " << WSPR::GetDefaultChannelIfNotValid(0) << endl;
    cout << "Channel 599: " << WSPR::GetDefaultChannelIfNotValid(599) << endl;
    cout << "Channel 600: " << WSPR::GetDefaultChannelIfNotValid(600) << endl;
    cout << "Channel 187: " << WSPR::GetDefaultChannelIfNotValid(187) << endl;

    cout << "MinuteList(\"20m\"):";
    for (const auto min : WSPR::GetMinuteListForBand("20m"))
    {
        cout << " " << (int)min;
    }
    cout << endl;

    cout << "DialFreq(20m): " << WSPR::GetDialFreqFromBandStr("20m") << endl;

    auto minuteList = WSPR::GetMinuteListForBand("20m");


    cout << "Channel Details for (\"20m\", 187):" << endl;
    WSPR::ChannelDetails cd = WSPR::GetChannelDetails("20m", 187);
    cout << "band    : " << cd.band << endl;
    cout << "channel : " << cd.channel << endl;
    cout << "id1     : " << cd.id1 << endl;
    cout << "id3     : " << cd.id3 << endl;
    cout << "id13    : " << cd.id13 << endl;
    cout << "min     : " << (int)cd.min << endl;
    cout << "lane    : " << (int)cd.lane << endl;
    cout << "freq    : " << cd.freq << endl;
    cout << "freqDial: " << cd.freqDial << endl;

    return 0;
}