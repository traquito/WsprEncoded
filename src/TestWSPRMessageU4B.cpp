#include <chrono>
#include <iostream>
#include <string>
#include <thread>
using namespace std;

#include "WSPRMessageU4B.h"


struct FieldDef
{
    string name;
    double lowValue;
    double highValue;
    double stepSize;
};

static FieldDef fieldDefList[] = {
    { .name = "grid5",       .lowValue = 'A', .highValue = 'X',   .stepSize = 1,    },
    { .name = "grid6",       .lowValue = 'A', .highValue = 'X',   .stepSize = 1,    },
    { .name = "altM",        .lowValue = 0,   .highValue = 21340, .stepSize = 1,    },
    { .name = "tempC",       .lowValue = -50, .highValue = 39,    .stepSize = 1,    },
    { .name = "voltage",     .lowValue = 3.0, .highValue = 4.95,  .stepSize = 0.05, },
    { .name = "speedKnots",  .lowValue = 0,   .highValue = 82,    .stepSize = 1,    },
    { .name = "gpsValid",    .lowValue = 0,   .highValue = 1,     .stepSize = 1,    },
};


static auto GetLow = [](const string &name){
    double retVal = 0;

    for (const auto &fieldDef : fieldDefList)
    {
        if (fieldDef.name == name)
        {
            retVal = fieldDef.lowValue;
        }
    }

    return retVal;
};

static auto GetHigh = [](const string &name){
    double retVal = 0;

    for (const auto &fieldDef : fieldDefList)
    {
        if (fieldDef.name == name)
        {
            retVal = fieldDef.highValue;
        }
    }

    return retVal;
};

static auto GetStep = [](const string &name){
    double retVal = 0;

    for (const auto &fieldDef : fieldDefList)
    {
        if (fieldDef.name == name)
        {
            retVal = fieldDef.stepSize;
        }
    }

    return retVal;
};

const double grid5Low  = GetLow("grid5");
const double grid5High = GetHigh("grid5");
const double grid5Step = GetStep("grid5");

const double grid6Low  = GetLow("grid6");
const double grid6High = GetHigh("grid6");
const double grid6Step = GetStep("grid6");

const double altMLow  = GetLow("altM");
const double altMHigh = GetHigh("altM");
const double altMStep = GetStep("altM");

const double tempCLow  = GetLow("tempC");
const double tempCHigh = GetHigh("tempC");
const double tempCStep = GetStep("tempC");

const double voltageLow  = GetLow("voltage");
const double voltageHigh = GetHigh("voltage");
const double voltageStep = GetStep("voltage");

const double speedKnotsLow  = GetLow("speedKnots");
const double speedKnotsHigh = GetHigh("speedKnots");
const double speedKnotsStep = GetStep("speedKnots");

const double gpsValidLow  = GetLow("gpsValid");
const double gpsValidHigh = GetHigh("gpsValid");
const double gpsValidStep = GetStep("gpsValid");



static uint64_t calcMax = 1;
static const uint8_t THREADS_MAX = 12;
static uint64_t calcsDone[THREADS_MAX];

void Test(string name, double grid5LowOverride, double grid5HighOverride, double grid5StepOverride)
{
    string id13 = "00";

    double grid5LowUse  = grid5LowOverride;
    double grid5HighUse = grid5HighOverride;
    double grid5StepUse = grid5StepOverride;

    uint32_t idx = atoi(name.c_str());
    calcsDone[idx] = 0;

    uint64_t &calcCount = calcsDone[idx];

    cout << "Test " << name << " using " << grid5LowUse << " " << grid5HighUse << " " << grid5StepUse << endl;

    string grid56 = "  ";
    for (char grid5 = grid5LowUse; grid5 <= grid5HighUse; grid5 += grid5StepUse)
    {
        grid56[0] = grid5;

        for (char grid6 = grid6Low; grid6 <= grid6High; grid6 += grid6Step)
        {
            grid56[1] = grid6;

            for (uint16_t altM = altMLow; altM <= altMHigh; altM += altMStep)
            {
                string call = WSPRMessageU4B::EncodeCallsign(id13, grid56, altM);

                for (int8_t tempC = tempCLow; tempC <= tempCHigh; tempC += tempCStep)
                {
                    for (double voltage = voltageLow; voltage <= voltageHigh; voltage += voltageStep)
                    {
                        for (uint8_t speedKnots = speedKnotsLow; speedKnots <= speedKnotsHigh; speedKnots += speedKnotsStep)
                        {
                            for (uint8_t gpsValid = gpsValidLow; gpsValid <= gpsValidHigh; gpsValid += gpsValidStep)
                            {
                                auto [grid, power] = WSPRMessageU4B::EncodeGridPower(tempC, voltage, speedKnots, gpsValid);

                                // cout << id13 << " " << grid56 << " " << altM << " " << (int)tempC << " " << voltage << " " << (int)speedKnots << " " << (int)gpsValid << endl;
                                // cout << call << " " << grid << " " << (int)power << endl;
                                // cout << endl;

                                // ++calcsDone;
                                ++calcCount;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Report()
{
    uint32_t count = 0;

    while (1)
    {
        ++count;

        uint64_t threadCalcsDone = 0;
        for (uint8_t i = 0; i < THREADS_MAX; ++i)
        {
            threadCalcsDone += calcsDone[i];
        }

        double pct = threadCalcsDone * 100.0 / calcMax;
        cout << count << ": " << endl;
        cout << fixed << setprecision(4) << pct << " % " << "done " << threadCalcsDone << " / " << calcMax << endl;

        uint32_t rate = threadCalcsDone / count;
        uint32_t rateMs = rate / 1000;
        cout << "Calculating at " << rate << " / sec, " << rateMs << " / ms" << endl;

        uint32_t hoursRemaining = 100.0 / pct * count / 60 / 60;
        cout << "Hours remaining: " << hoursRemaining << endl;

        cout << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (threadCalcsDone >= calcMax)
        {
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    for (const auto &fieldDef : fieldDefList)
    {
        calcMax *= ((fieldDef.highValue - fieldDef.lowValue) / fieldDef.stepSize) + 1;
    }

    cout << "Calculations required: " << calcMax << endl;

    unsigned int numCores = thread::hardware_concurrency();
    cout << "Number of cores: " << numCores << endl;

    thread t1 (Test, to_string(1),  'A', 'B', 1);
    thread t2 (Test, to_string(2),  'C', 'D', 1);
    thread t3 (Test, to_string(3),  'E', 'F', 1);
    thread t4 (Test, to_string(4),  'G', 'H', 1);
    thread t5 (Test, to_string(5),  'I', 'J', 1);
    thread t6 (Test, to_string(6),  'K', 'L', 1);
    thread t7 (Test, to_string(7),  'M', 'N', 1);
    thread t8 (Test, to_string(8),  'O', 'P', 1);
    thread t9 (Test, to_string(9),  'Q', 'R', 1);
    thread t10(Test, to_string(10), 'S', 'U', 1);   // 2
    thread t11(Test, to_string(11), 'V', 'X', 1);   // 2
    thread tReport(Report);

    tReport.join();


    return 0;
}