#include <iostream>
using namespace std;

#include "WsprMessageRegularType1.h"


int main(int argc, char *argv[])
{
    WsprMessageRegularType1 msg;

    msg.SetCallsign("DOUG");
    msg.SetGrid("FN20");
    msg.SetPowerDbm(13);

    cout << "Testing" << endl;
    cout << msg.GetCallsign() << " " << msg.GetGrid() << " " << (int)msg.GetPowerDbm() << endl;

    return 0;
}