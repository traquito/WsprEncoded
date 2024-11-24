#include <array>
#include <iostream>
using namespace std;

#include "WsprUtl.h"


template<typename T, size_t SIZE>
void PrintArray(const array<T, SIZE> &arr)
{
    string sep = "";

    for (const auto &a : arr)
    {
        cout << sep << (int)a;

        sep = " ";
    }
}

template<typename T, size_t SIZE>
array<T, SIZE> TestRotate(array<T, SIZE> arr, int count)
{
    cout << "Rotate " << count << ":" << endl;
    cout << "Before: ";
    PrintArray(arr);
    cout << endl;

    auto arrNew = Rotate(arr, count);

    cout << "After : ";
    PrintArray(arrNew);
    cout << endl;

    return arrNew;
}

int main(int argc, char *argv[])
{
    array<uint8_t, 5> minuteList = { 8, 0, 2, 4, 6 };

    TestRotate(minuteList, -6); cout << endl;
    TestRotate(minuteList, -5); cout << endl;
    TestRotate(minuteList, -4); cout << endl;
    TestRotate(minuteList, -3); cout << endl;
    TestRotate(minuteList, -2); cout << endl;
    TestRotate(minuteList, -1); cout << endl;
    TestRotate(minuteList, 0); cout << endl;
    TestRotate(minuteList, 1); cout << endl;
    TestRotate(minuteList, 2); cout << endl;
    TestRotate(minuteList, 3); cout << endl;
    TestRotate(minuteList, 4); cout << endl;
    TestRotate(minuteList, 5); cout << endl;
    TestRotate(minuteList, 6); cout << endl;

    return 0;
}