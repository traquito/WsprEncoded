#pragma once


template <typename T1, typename T2>
struct InputTest
{
    T1     input;
    T2     expectedRetVal;
    string comment;
};

template <typename T1, typename T2>
bool CheckErr(const InputTest<T1, T2> &test, T2 testRetVal)
{
    bool retVal = true;

    if (testRetVal != test.expectedRetVal)
    {
        retVal = false;

        cout << "ERR: Got " << testRetVal << ", but expected " << test.expectedRetVal << " (" << test.comment << ")" << endl;
    }

    return retVal;
}