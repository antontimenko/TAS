#include "TSInteger.h"

#include <limits>

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

    if (includeSign && (*this > 0_I))
        str = "+" + str;

    return str;
}
