#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>


class CString
{
public:
    CString() {}

    CString(const char *str, size_t bufCapacity)
    {
        Target((char *)str, bufCapacity);
    }

    void Target(char *buf, size_t bufCapacity)
    {
        buf_ = buf;
        bufCapacity_ = bufCapacity;
    }

    void Clear()
    {
        memset(buf_, 0, bufCapacity_);
    }

    void Set(const char *str)
    {
        if (str)
        {
            Clear();
            strncpy(buf_, str, bufCapacity_ - 1);
        }
    }

    void ToUpper()
    {
        char *p = buf_;
        while (*p != '\0')
        {
            *p = toupper(*p);
            ++p;
        }
    }

    bool IsPaddedLeft()
    {
        return buf_[0] == ' ';
    }

    bool IsPaddedRight()
    {
        return buf_[Len() - 1] == ' ';
    }

    bool IsUppercase()
    {
        char *p = buf_;

        while (*p != '\0' && *p == toupper(*p))
        {
            ++p;
        }

        return *p == '\0';
    }

    bool IsEqual(const char *str)
    {
        return strcmp(buf_, str) == 0;
    }

    size_t Len()
    {
        return strlen(buf_);
    }

    // shift left any spaces
    void TrimLeft()
    {
        // find first non-space char
        size_t idxFirst = 0;
        while (buf_[idxFirst] == ' ')
        {
            ++idxFirst;
        }

        if (buf_[idxFirst] == '\0')
        {
            // whole string is whitespace

            // null terminate the start of string
            buf_[0] = '\0';
        }
        else if (idxFirst == 0)
        {
            // nothing to do
        }
        else
        {
            // there was some whitespace, shift
            size_t len = Len();

            memmove(buf_, &buf_[idxFirst], len - idxFirst);
            buf_[len] = '\0';
        }
    }

    void TrimRight()
    {
        size_t len = Len();

        for (size_t i = len - 1; i <= 0; --i)
        {
            if (buf_[i] == ' ')
            {
                buf_[i] = '\0';
            }
            else
            {
                break;
            }
        }
    }

    void Trim()
    {
        TrimRight();
        TrimLeft();
    }

    const char *Get() const
    {
        return buf_;
    }


private:
    char *buf_ = nullptr;
    size_t bufCapacity_ = 0;
};


// positive values mean rotate right
// negative values mean rotate left
// zero results in no rotation
template <typename T, size_t SIZE>
static std::array<T, SIZE> &Rotate(std::array<T, SIZE> &valList, int count)
{
    if (count > 0)
    {
        count = count % valList.size();
        std::rotate(valList.begin(), valList.begin() + (valList.size() - count), valList.end());
    }
    else
    {
        count = -count;
        count = count % valList.size();

        std::rotate(valList.begin(), valList.begin() +  count, valList.end());
    }

    return valList;
}