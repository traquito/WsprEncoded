#include <iostream>
using namespace std;

#include "WsprChannelMap.h"



#include "../WSPR.h"
void CompareOldToNew()
{
    for (auto &bd : Wspr::GetBandDataList())
    {
        for (int i = 0; i < 600; ++i)
        {
            WSPR::ChannelDetails           cdOld = WSPR::GetChannelDetails(bd.band, i);
            WsprChannelMap::ChannelDetails cdNew = WsprChannelMap::GetChannelDetails(bd.band, i);

            bool ok = true;
            if (cdOld.band != cdNew.band)
            {
                ok = false;

                cout << "ERR: Difference in band, old(" << cdOld.band << "), " << ", new(" << cdNew.band << ")" << endl;
            }
            if (cdOld.channel != cdNew.channel)
            {
                ok = false;

                cout << "ERR: Difference in channel, old(" << cdOld.channel << "), " << ", new(" << cdNew.channel << ")" << endl;
            }
            if (cdOld.id1 != cdNew.id1)
            {
                ok = false;

                cout << "ERR: Difference in id1, old(" << cdOld.id1 << "), " << ", new(" << cdNew.id1 << ")" << endl;
            }
            if (cdOld.id3 != cdNew.id3)
            {
                ok = false;

                cout << "ERR: Difference in id3, old(" << cdOld.id3 << "), " << ", new(" << cdNew.id3 << ")" << endl;
            }
            if (cdOld.id13 != cdNew.id13)
            {
                ok = false;

                cout << "ERR: Difference in id13, old(" << cdOld.id13 << "), " << ", new(" << cdNew.id13 << ")" << endl;
            }
            if (cdOld.min != cdNew.min)
            {
                ok = false;

                cout << "ERR: Difference in min, old(" << cdOld.min << "), " << ", new(" << cdNew.min << ")" << endl;
            }
            if (cdOld.lane != cdNew.lane)
            {
                ok = false;

                cout << "ERR: Difference in lane, old(" << cdOld.lane << "), " << ", new(" << cdNew.lane << ")" << endl;
            }
            if (cdOld.freq != cdNew.freq)
            {
                ok = false;

                cout << "ERR: Difference in freq, old(" << cdOld.freq << "), " << ", new(" << cdNew.freq << ")" << endl;
            }
            if (cdOld.freqDial != cdNew.freqDial)
            {
                ok = false;

                cout << "ERR: Difference in freqDial, old(" << cdOld.freqDial << "), " << ", new(" << cdNew.freqDial << ")" << endl;
            }

            if (ok == false)
            {
                cout << endl;
            }
        }
    }
}



int main(int argc, char *argv[])
{
    cout << "Channel   0: " << WsprChannelMap::GetDefaultChannelIfNotValid(0) << endl;
    cout << "Channel 599: " << WsprChannelMap::GetDefaultChannelIfNotValid(599) << endl;
    cout << "Channel 600: " << WsprChannelMap::GetDefaultChannelIfNotValid(600) << endl;
    cout << "Channel 187: " << WsprChannelMap::GetDefaultChannelIfNotValid(187) << endl;

    cout << "MinuteList(\"20m\"):";
    for (const auto min : WsprChannelMap::GetMinuteListForBand("20m"))
    {
        cout << " " << (int)min;
    }
    cout << endl;

    cout << "DialFreq(20m): " << Wspr::GetDialFreqFromBandStr("20m") << endl;

    auto minuteList = WsprChannelMap::GetMinuteListForBand("20m");


    cout << "Channel Details for (\"20m\", 187):" << endl;
    WsprChannelMap::ChannelDetails cd = WsprChannelMap::GetChannelDetails("20m", 187);
    cout << "band    : " << cd.band << endl;
    cout << "channel : " << cd.channel << endl;
    cout << "id1     : " << cd.id1 << endl;
    cout << "id3     : " << cd.id3 << endl;
    cout << "id13    : " << cd.id13 << endl;
    cout << "min     : " << (int)cd.min << endl;
    cout << "lane    : " << (int)cd.lane << endl;
    cout << "freq    : " << cd.freq << endl;
    cout << "freqDial: " << cd.freqDial << endl;

    CompareOldToNew();

    return 0;
}