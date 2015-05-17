#include "TSInteger.h"

#include <limits>
#include <cstdint>

using std::experimental::optional;
using std::experimental::nullopt;

constexpr TSInt TSInt_MIN = std::numeric_limits<TSInt>::min();
constexpr TSInt TSInt_MAX = std::numeric_limits<TSInt>::max();
constexpr TSUInt TSUInt_MAX = std::numeric_limits<TSUInt>::max();

bool TSInteger::operator==(const TSInteger &num) const
{
    if (isSigned && num.isSigned)
        return (TSInt)val == (TSInt)num.val;
    else if (!isSigned && num.isSigned)
        return (((TSUInt)val > TSInt_MAX) || ((TSInt)num.val < 0)) ? false : ((TSUInt)val == (TSUInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((TSInt)val < 0) || ((TSUInt)num.val > TSInt_MAX)) ? false : ((TSUInt)val == (TSUInt)num.val);
    else
        return (TSUInt)val == (TSUInt)num.val;
}

bool TSInteger::operator!=(const TSInteger &num) const
{
    if (isSigned && num.isSigned)
        return (TSInt)val != (TSInt)num.val;
    else if (!isSigned && num.isSigned)
        return (((TSUInt)val > TSInt_MAX) || ((TSInt)num.val < 0)) ? true : ((TSUInt)val != (TSUInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((TSInt)val < 0) || ((TSUInt)num.val > TSInt_MAX)) ? true : ((TSUInt)val != (TSUInt)num.val);
    else
        return (TSUInt)val != (TSUInt)num.val;
}

bool TSInteger::operator<(const TSInteger &num) const
{
    if (isSigned && num.isSigned)
        return (TSInt)val < (TSInt)num.val;
    else if (!isSigned && num.isSigned)
        return (((TSUInt)val > TSInt_MAX) || ((TSInt)num.val < 0)) ? false : ((TSUInt)val < (TSUInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((TSInt)val < 0) || ((TSUInt)num.val > TSInt_MAX)) ? true : ((TSUInt)val < (TSUInt)num.val);
    else
        return (TSUInt)val < (TSUInt)num.val;
}

bool TSInteger::operator>(const TSInteger &num) const
{
    if (isSigned && num.isSigned)
        return (TSInt)val > (TSInt)num.val;
    else if (!isSigned && num.isSigned)
        return (((TSUInt)val > TSInt_MAX) || ((TSInt)num.val < 0)) ? true : ((TSUInt)val > (TSUInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((TSInt)val < 0) || ((TSUInt)num.val > TSInt_MAX)) ? false : ((TSUInt)val > (TSUInt)num.val);
    else
        return (TSUInt)val > (TSUInt)num.val;
}

bool TSInteger::operator<=(const TSInteger &num) const
{
    if (isSigned && num.isSigned)
        return (TSInt)val <= (TSInt)num.val;
    else if (!isSigned && num.isSigned)
        return (((TSUInt)val > TSInt_MAX) || ((TSInt)num.val < 0)) ? false : ((TSUInt)val <= (TSUInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((TSInt)val < 0) || ((TSUInt)num.val > TSInt_MAX)) ? true : ((TSUInt)val <= (TSUInt)num.val);
    else
        return (TSUInt)val <= (TSUInt)num.val;
}

bool TSInteger::operator>=(const TSInteger &num) const
{
    if (isSigned && num.isSigned)
        return (TSInt)val >= (TSInt)num.val;
    else if (!isSigned && num.isSigned)
        return (((TSUInt)val > TSInt_MAX) || ((TSInt)num.val < 0)) ? true : ((TSUInt)val >= (TSUInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((TSInt)val < 0) || ((TSUInt)num.val > TSInt_MAX)) ? false : ((TSUInt)val >= (TSUInt)num.val);
    else
        return (TSUInt)val >= (TSUInt)num.val;
}

TSInteger TSInteger::operator+(const TSInteger &num) const
{
    if (isSigned && ((TSInt)val < 0))
    {
        if (num.isSigned && ((TSInt)num.val < 0))
        {
            if ((TSInt_MIN - (TSInt)num.val) > (TSInt)val)
                throw std::overflow_error("overflow (too low)");

            return (TSInt)val + (TSInt)num.val;
        }
        else if (!num.isSigned && ((TSUInt)num.val > TSInt_MAX))
            return (TSUInt)num.val - (TSUInt)(-(TSInt)val);
        else
            return (TSInt)val + (TSInt)num.val;
    }
    else if (!isSigned && ((TSUInt)val > TSInt_MAX))
    {
        if (num.isSigned && ((TSInt)num.val < 0))
            return (TSUInt)val - (TSUInt)(-(TSInt)num.val);
        else if (!num.isSigned && ((TSUInt)num.val > TSInt_MAX))
            throw std::overflow_error("overflow (too high)");
        else
        {
            if ((TSUInt_MAX - (TSUInt)num.val) < (TSUInt)val)
                throw std::overflow_error("overflow (too high)");

            return (TSUInt)val + (TSUInt)num.val;
        }
    }
    else
    {
        if (num.isSigned && ((TSInt)num.val < 0))
            return (TSInt)val + (TSInt)num.val;
        else if (!num.isSigned && ((TSUInt)num.val > TSInt_MAX))
        {
            if ((TSUInt_MAX - (TSUInt)num.val) < (TSUInt)val)
                throw std::overflow_error("overflow (too high)");

            return (TSUInt)val + (TSUInt)num.val;
        }
        else
            return (TSUInt)val + (TSUInt)num.val;
    }
}

TSInteger TSInteger::operator-(const TSInteger &num) const
{
    if (isSigned && ((TSInt)val < 0))
    {
        if (num.isSigned && ((TSInt)num.val < 0))
            return (TSInt)val - (TSInt)num.val;
        else if (!num.isSigned && ((TSUInt)num.val > TSInt_MAX))
            throw std::overflow_error("overflow (too low)");
        else
        {
            if ((TSInt_MIN + (TSInt)num.val) > (TSInt)val)
                throw std::overflow_error("overflow (too low)");

            return (TSInt)val - (TSInt)num.val;
        }
    }
    else if (!isSigned && ((TSUInt)val > TSInt_MAX))
    {
        if (num.isSigned && ((TSInt)num.val < 0))
        {
            if ((TSUInt_MAX - (TSUInt)(-(TSInt)num.val)) < (TSUInt)val)
                throw std::overflow_error("overflow (too high)");

            return (TSUInt)val + (TSUInt)(-(TSInt)num.val);
        }
        else if (!num.isSigned && ((TSUInt)num.val > TSInt_MAX))
        {
            if ((TSUInt)val > (TSUInt)num.val)
                return (TSInt)((TSUInt)val - (TSUInt)num.val);
            else
                return -(TSInt)((TSUInt)num.val - (TSUInt)val);
        }
        else
            return (TSUInt)val - (TSUInt)num.val;
    }
    else
    {
        if (num.isSigned && ((TSInt)num.val < 0))
            return (TSUInt)val + (TSUInt)(-(TSInt)num.val);
        else if (!num.isSigned && ((TSUInt)num.val > TSInt_MAX))
        {
            if ((TSUInt)(-TSInt_MIN) < ((TSUInt)num.val - (TSUInt)val))
                throw std::overflow_error("overflow (too low)");

            return -(TSInt)((TSUInt)num.val - (TSUInt)val);
        }
        else
            return (TSInt)val - (TSInt)num.val;
    }
}

TSInteger TSInteger::operator+() const
{
    return *this;
}

TSInteger TSInteger::operator-() const
{
    if (!isSigned && ((TSUInt)val > (TSUInt)(-TSInt_MIN)))
        throw std::overflow_error("overflow (too big to negate)");

    return -(TSInt)val;
}

TSInteger TSInteger::operator*(const TSInteger &num) const
{
    bool isResultNegative = (isSigned && ((TSInt)val < 0)) != (num.isSigned && ((TSInt)num.val < 0));

    TSUInt mult1 = isSigned ? (TSUInt)llabs((TSInt)val) : (TSUInt)val;
    TSUInt mult2 = num.isSigned ? (TSUInt)llabs((TSInt)num.val) : (TSUInt)num.val;

    TSUInt res = mult1 * mult2;

    if ((mult1 != 0) && ((res / mult1) != mult2))
        throw std::overflow_error("overflow (too large multipliers)");

    if (isResultNegative)
    {
        if (res > (TSUInt)(-TSInt_MIN))
            throw std::overflow_error("overflow (too large multipliers)");

        return -(TSInt)res;
    }
    else
        return (TSUInt)res;
}

TSInteger TSInteger::operator/(const TSInteger &num) const
{
    bool isResultNegative = (isSigned && ((TSInt)val < 0)) != (num.isSigned && ((TSInt)num.val < 0));

    TSUInt div1 = isSigned ? (TSUInt)llabs((TSInt)val) : (TSUInt)val;
    TSUInt div2 = num.isSigned ? (TSUInt)llabs((TSInt)num.val) : (TSUInt)num.val;

    if (div2 == 0)
        throw std::overflow_error("overflow (division by zero)");

    TSUInt res = div1 / div2;

    if (isResultNegative)
    {
        if (res > (TSUInt)(-TSInt_MIN))
            throw std::overflow_error("overflow (too large divident)");

        return -(TSInt)res;
    }
    else
        return (TSUInt)res;
}

std::string TSInteger::str(bool includeSign) const
{
    std::string str =  isSigned ? std::to_string((TSInt)val) : std::to_string((TSUInt)val);

    if (includeSign && (*this > 0))
        str = "+" + str;

    return str;
}

optional<TSInteger::Size> TSInteger::sizeSigned() const
{
    if ((*this >= INT8_MIN) && (*this <= INT8_MAX))
        return Size::S_8;
    else if ((*this >= INT16_MIN) && (*this <= INT16_MAX))
        return Size::S_16;
    else if ((*this >= INT32_MIN) && (*this <= INT32_MAX))
        return Size::S_32;
    else if ((*this >= INT64_MIN) && (*this <= INT64_MAX))
        return Size::S_64;
    else
        return nullopt;
}

optional<TSInteger::Size> TSInteger::sizeUnsigned() const
{
    if ((*this >= 0) && (*this <= UINT8_MAX))
        return Size::S_8;
    else if ((*this >= 0) && (*this <= UINT16_MAX))
        return Size::S_16;
    else if ((*this >= 0) && (*this <= UINT32_MAX))
        return Size::S_32;
    else if ((*this >= 0) && (*this <= UINT64_MAX))
        return Size::S_64;
    else
        return nullopt;
}

TSInteger::Size TSInteger::sizeAny() const
{
    if ((*this >= INT8_MIN) && (*this <= UINT8_MAX))
        return Size::S_8;
    else if ((*this >= INT16_MIN) && (*this <= UINT16_MAX))
        return Size::S_16;
    else if ((*this >= INT32_MIN) && (*this <= UINT32_MAX))
        return Size::S_32;
    else
        return Size::S_64;
}

using std::vector;
typedef unsigned char uchar;

vector<uchar> TSInteger::getCharArraySigned(Size size) const
{
    vector<uchar> res;

    if (size == Size::S_8)
    {
        int8_t vecVal = (int8_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int8_t); ++i)
            res.push_back(p[i]);
    }
    else if (size == Size::S_16)
    {
        int16_t vecVal = (int16_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int16_t); ++i)
            res.push_back(p[i]);
    }
    else if (size == Size::S_32)
    {
        int32_t vecVal = (int32_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int32_t); ++i)
            res.push_back(p[i]);
    }
    else
    {
        int64_t vecVal = (int64_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int64_t); ++i)
            res.push_back(p[i]);
    }

    return res;
}

vector<uchar> TSInteger::getCharArrayUnsigned(Size size) const
{
    vector<uchar> res;

    if (size == Size::S_8)
    {
        uint8_t vecVal = (uint8_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint8_t); ++i)
            res.push_back(p[i]);
    }
    else if (size == Size::S_16)
    {
        uint16_t vecVal = (uint16_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint16_t); ++i)
            res.push_back(p[i]);
    }
    else if (size == Size::S_32)
    {
        uint32_t vecVal = (uint32_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint32_t); ++i)
            res.push_back(p[i]);
    }
    else
    {
        uint64_t vecVal = (uint64_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint64_t); ++i)
            res.push_back(p[i]);
    }

    return res;
}

TSInteger TSInteger::getMaxValSigned(Size size)
{
    switch (size)
    {
    case Size::S_8:
        return INT8_MAX;
    case Size::S_16:
        return INT16_MAX;
    case Size::S_32:
        return INT32_MAX;
    case Size::S_64:
        return INT64_MAX;
    }
}

TSInteger TSInteger::getMaxValUnsigned(Size size)
{
    switch (size)
    {
    case Size::S_8:
        return UINT8_MAX;
    case Size::S_16:
        return UINT16_MAX;
    case Size::S_32:
        return UINT32_MAX;
    case Size::S_64:
        return UINT64_MAX;
    }
}

TSInteger TSInteger::getMaxValAny(Size size)
{
    return getMaxValUnsigned(size);
}

TSInteger::Size TSInteger::nextSize(Size size)
{
    switch (size)
    {
    case Size::S_8:
        return Size::S_16;
    case Size::S_16:
        return Size::S_32;
    case Size::S_32:
        return Size::S_64;
    case Size::S_64:
        return Size::S_64;
    }
}
