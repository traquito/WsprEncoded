#include <chrono>
#include <iostream>
#include <string>
#include <thread>
using namespace std;

#include "WSPRMessageU4B.h"
#include "WSPRMessageU4BDecoder.h"
#include "./new/WsprMessageTelemetryBasic.h"


struct FieldDef
{
    string name;
    double lowValue;
    double highValue;
    double stepSize;
    double testStepSize;
};

static FieldDef fieldDefList[] = {
    { .name = "grid5",       .lowValue = 'A', .highValue = 'X',   .stepSize = 1,    .testStepSize = 1,    },
    { .name = "grid6",       .lowValue = 'A', .highValue = 'X',   .stepSize = 1,    .testStepSize = 1,    },
    { .name = "altM",        .lowValue = 0,   .highValue = 21340, .stepSize = 20,   .testStepSize = 1,    },
    { .name = "tempC",       .lowValue = -50, .highValue = 39,    .stepSize = 1,    .testStepSize = 1,    },
    { .name = "voltage",     .lowValue = 3.0, .highValue = 4.95,  .stepSize = 0.05, .testStepSize = 0.05, },
    { .name = "speedKnots",  .lowValue = 0,   .highValue = 82,    .stepSize = 2,    .testStepSize = 1,    },
    { .name = "gpsValid",    .lowValue = 0,   .highValue = 1,     .stepSize = 1,    .testStepSize = 1,    },
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

static auto GetTestStep = [](const string &name){
    double retVal = 0;

    for (const auto &fieldDef : fieldDefList)
    {
        if (fieldDef.name == name)
        {
            retVal = fieldDef.testStepSize;
        }
    }

    return retVal;
};

const double grid5Low      = GetLow("grid5");
const double grid5High     = GetHigh("grid5");
const double grid5Step     = GetStep("grid5");
const double grid5TestStep = GetTestStep("grid5");

const double grid6Low      = GetLow("grid6");
const double grid6High     = GetHigh("grid6");
const double grid6Step     = GetStep("grid6");
const double grid6TestStep = GetTestStep("grid6");

const double altMLow      = GetLow("altM");
const double altMHigh     = GetHigh("altM");
const double altMStep     = GetStep("altM");
const double altMTestStep = GetTestStep("altM");

const double tempCLow      = GetLow("tempC");
const double tempCHigh     = GetHigh("tempC");
const double tempCStep     = GetStep("tempC");
const double tempCTestStep = GetTestStep("tempC");

const double voltageLow      = GetLow("voltage");
const double voltageHigh     = GetHigh("voltage");
const double voltageStep     = GetStep("voltage");
const double voltageTestStep = GetTestStep("voltage");

const double speedKnotsLow      = GetLow("speedKnots");
const double speedKnotsHigh     = GetHigh("speedKnots");
const double speedKnotsStep     = GetStep("speedKnots");
const double speedKnotsTestStep = GetTestStep("speedKnots");

const double gpsValidLow      = GetLow("gpsValid");
const double gpsValidHigh     = GetHigh("gpsValid");
const double gpsValidStep     = GetStep("gpsValid");
const double gpsValidTestStep = GetTestStep("gpsValid");


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

        for (char grid6 = grid6Low; grid6 <= grid6High; grid6 += grid6TestStep)
        {
            grid56[1] = grid6;

            for (uint16_t altM = altMLow; altM <= altMHigh; altM += altMTestStep)
            {
                string call = WSPRMessageU4B::EncodeCallsign(id13, grid56, altM);

                auto [grid56Decoded, altMDecoded] = WSPRMessageU4BDecoder::DecodeU4BCall(call);

                if (grid56Decoded != grid56)
                {
                    cout << "grid56 decode error" << endl;
                }
                
                if (altMDecoded != altM)
                {
                    // check if altM got snapped
                    uint16_t altMSnapped = round(altM / altMStep) * (uint32_t)altMStep;
                    if (altMDecoded != altMSnapped)
                    {
                        cout << "altM decode error" << endl;
                        cout << "Input : " << id13 << " " << grid56        << " " << altM        << endl;
                        cout << "Output: " << "  " << " " << grid56Decoded << " " << altMDecoded << endl;
                        cout << "Snap'd: " << "  " << " " << "  "          << " " << altMSnapped << endl;

                        cout << endl;
                    }
                }

                for (int8_t tempC = tempCLow; tempC <= tempCHigh; tempC += tempCTestStep)
                {
                    for (double voltage = voltageLow; voltage <= voltageHigh; voltage += voltageTestStep)
                    {
                        for (uint8_t speedKnots = speedKnotsLow; speedKnots <= speedKnotsHigh; speedKnots += speedKnotsTestStep)
                        {
                            for (uint8_t gpsValid = gpsValidLow; gpsValid <= gpsValidHigh; gpsValid += gpsValidTestStep)
                            {
                                auto [grid, power] = WSPRMessageU4B::EncodeGridPower(tempC, voltage, speedKnots, gpsValid);

                                auto [ telemetryId, tempCDecoded, voltageDecoded, speedKnotsDecoded, gpsValidDecoded ] = WSPRMessageU4BDecoder::DecodeU4BGridPower(grid, power);


                                // quick hack to just throw in the new encoder/decoder
                                {
                                    WsprMessageTelemetryBasic tb;

                                    // ENCODE
                                    tb.SetAltitudeMeters(altM);
                                    tb.SetTemperatureCelsius(tempC);
                                    tb.SetVoltageVolts(voltage);
                                    tb.SetSpeedKnots(speedKnots);
                                    tb.SetGpsIsValid(gpsValid);
                                    tb.SetGrid56(grid56.c_str());
                                    
                                    tb.SetId13(id13.c_str());
                                    tb.Encode();

                                    const char *tbCall = tb.GetCallsign();
                                    const char *tbGrid = tb.GetGrid4();
                                    uint8_t tbPowerDbm = tb.GetPowerDbm();

                                    if (call != tbCall)
                                    {
                                        cout << "Call: OH SHITTTTTT" << endl;
                                    }

                                    if (grid != tbGrid)
                                    {
                                        cout << "Grid: OH SHITTTTTT" << endl;
                                    }

                                    if (power != tbPowerDbm)
                                    {
                                        cout << "pDbm: OH SHITTTTTT" << endl;
                                    }
                                }

                                {
                                    WsprMessageTelemetryBasic tb;

                                    if (tb.SetCallsign(call.c_str()) == false)
                                    {
                                        cout << "Barfed on setting callsign to " << call.c_str() << endl;
                                    }

                                    if (tb.SetGrid4(grid.c_str()) == false)
                                    {
                                        cout << "Barfed on setting grid to " << grid.c_str() << endl;
                                    }

                                    if (tb.SetPowerDbm(power) == false)
                                    {
                                        cout << "Barfed on setting power to " << (int)power << endl;
                                    }

                                    bool decodeOk = tb.Decode();

                                    const char *tbGrid56     = tb.GetGrid56();
                                    uint16_t    tbAltM       = tb.GetAltitudeMeters();
                                    int8_t      tbTempC      = tb.GetTemperatureCelsius();
                                    double      tbVolts      = tb.GetVoltageVolts();
                                    uint8_t     tbSpeedKnots = tb.GetSpeedKnots();
                                    bool        tbGpsValid   = tb.GetGpsIsValid();

                                    bool tbOk = true;
                                    if (decodeOk == false)
                                    {
                                        tbOk = false;
                                        cout << "Decode: OH SHITTTTTT" << endl;
                                    }

                                    if (grid56Decoded != tbGrid56)
                                    {
                                        tbOk = false;
                                        cout << "grid56: OH SHITTTTTT" << endl;
                                    }

                                    if (altMDecoded != tbAltM)
                                    {
                                        tbOk = false;
                                        cout << "altM: OH SHITTTTTT" << endl;
                                    }

                                    if (tempCDecoded != tbTempC)
                                    {
                                        tbOk = false;
                                        cout << "tempC: OH SHITTTTTT" << endl;
                                    }

                                    if (voltageDecoded != tbVolts)
                                    {
                                        tbOk = false;
                                        cout << "voltage: OH SHITTTTTT" << endl;
                                    }

                                    if (speedKnotsDecoded != tbSpeedKnots)
                                    {
                                        tbOk = false;
                                        cout << "speedKnots: OH SHITTTTTT" << endl;
                                    }

                                    if (gpsValidDecoded != tbGpsValid)
                                    {
                                        tbOk = false;
                                        cout << "gpsValid: OH SHITTTTTT" << endl;
                                    }

                                    if (tbOk == false)
                                    {
                                        cout << "Input : " << (int)tempC        << " " << voltage        << " " << (int)speedKnots        << " " << (int)gpsValid        << endl;
                                        cout << "Output: " << (int)tbTempC << " " << tbVolts << " " << (int)tbSpeedKnots << " " << (int)tbGpsValid << endl;
                                        cout << endl;
                                    }
                                }




                                bool err = false;
                                if (tempCDecoded != tempC)
                                {
                                    err = true;
                                    cout << "tempC decode error" << endl;
                                }

                                if (voltageDecoded != voltage)
                                {
                                    // I don't remember enough about floats to nail down what specifically is the
                                    // issue here, but it's clearly a floating point manipulation issue if it falls
                                    // in this range.
                                    //
                                    // https://stackoverflow.com/questions/17333/how-do-you-compare-float-and-double-while-accounting-for-precision-loss
                                    //
                                    // the epsilon below is the smallest tick movable in a double.
                                    // through testing, I'm seeing the diff within multiples of that
                                    // eg:
                                    // diff     : 2.66454e-15
                                    // abs(diff): 2.66454e-15
                                    // epsilon  : 2.22045e-16
                                    // epsilon2x: 4.44089e-16
                                    // 
                                    // that's how many zeroes. they're the same number.
                                    //
                                    // just see if they're within a billionth of one another, good enough
                                    if (fabs(voltageDecoded - voltage) > (0.000'000'000'001))
                                    {
                                        err = true;
                                        cout << "voltage decode error" << endl;
                                        cout << "  diff     : " << (voltageDecoded - voltage) << endl;
                                        cout << "  abs(diff): " << fabs(voltageDecoded - voltage) << endl;
                                        cout << "  epsilon  : " << std::numeric_limits<double>::epsilon() << endl;
                                        cout << "  epsilon2x: " << std::numeric_limits<double>::epsilon() * 2 << endl;
                                    }
                                }

                                if (speedKnotsDecoded != speedKnots)
                                {
                                    // check if speedKnots got snapped
                                    uint16_t speedKnotsSnapped = round(speedKnots / speedKnotsStep) * (uint32_t)speedKnotsStep;
                                    if (speedKnotsDecoded != speedKnotsSnapped)
                                    {
                                        err = true;
                                        cout << "speedKnots decode error" << endl;
                                        cout << "Snapped: " << speedKnotsSnapped << endl;
                                    }
                                }

                                if (gpsValidDecoded != gpsValid)
                                {
                                    err = true;
                                    cout << "gpsValid decode error" << endl;
                                }

                                if (telemetryId != 1)
                                {
                                    err = true;
                                    cout << "telemetryId says non-standard telemetry error!" << endl;
                                }

                                if (err)
                                {
                                    cout << "Input : " << (int)tempC        << " " << voltage        << " " << (int)speedKnots        << " " << (int)gpsValid        << endl;
                                    cout << "Output: " << (int)tempCDecoded << " " << voltageDecoded << " " << (int)speedKnotsDecoded << " " << (int)gpsValidDecoded << endl;
                                    cout << "Via   : " << grid << " " << (int)power << endl;

                                    cout << "Power Val: " << (int)DecodePowerToNum(power) << endl;

                                    cout << endl;
                                }
                                

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
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        ++count;

        uint64_t threadCalcsDone = 0;
        for (uint8_t i = 0; i < THREADS_MAX; ++i)
        {
            threadCalcsDone += calcsDone[i];
        }

        double pct = threadCalcsDone * 100.0 / calcMax;
        cout << count << ": " << endl;
        // cout << fixed << setprecision(4) << pct << " % " << "done " << threadCalcsDone << " / " << calcMax << endl;
        cout << pct << " % " << "done " << threadCalcsDone << " / " << calcMax << endl;

        uint32_t rate = threadCalcsDone / count;
        uint32_t rateMs = rate / 1000;
        cout << "Calculating at " << rate << " / sec, " << rateMs << " / ms" << endl;

        uint32_t hoursRemaining = 100.0 / pct * count / 60 / 60;
        cout << "Hours remaining: " << hoursRemaining << endl;

        cout << endl;

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
        calcMax *= ((fieldDef.highValue - fieldDef.lowValue) / fieldDef.testStepSize) + 1;
    }

    cout << "Calculations required: " << calcMax << endl;

    unsigned int numCores = thread::hardware_concurrency();
    cout << "Number of cores: " << numCores << endl;

    cout << endl;
    cout << endl;
    cout << endl;

    thread t1 (Test, to_string(1),  'A', 'B', grid5TestStep);
    thread t2 (Test, to_string(2),  'C', 'D', grid5TestStep);
    thread t3 (Test, to_string(3),  'E', 'F', grid5TestStep);
    thread t4 (Test, to_string(4),  'G', 'H', grid5TestStep);
    thread t5 (Test, to_string(5),  'I', 'J', grid5TestStep);
    thread t6 (Test, to_string(6),  'K', 'L', grid5TestStep);
    thread t7 (Test, to_string(7),  'M', 'N', grid5TestStep);
    thread t8 (Test, to_string(8),  'O', 'P', grid5TestStep);
    thread t9 (Test, to_string(9),  'Q', 'R', grid5TestStep);
    thread t10(Test, to_string(10), 'S', 'U', grid5TestStep);   // 2
    thread t11(Test, to_string(11), 'V', 'X', grid5TestStep);   // 2
    thread tReport(Report);

    tReport.join();


    return 0;
}