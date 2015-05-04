#ifndef _TSINTEGER_H_
#define _TSINTEGER_H_

#include <stdexcept>
#include <string>

typedef long long int TSInt;
typedef unsigned long long int TSUInt;

class TSInteger
{
public:
    constexpr TSInteger() :
        TSInteger((TSUInt)0)
    {}

    constexpr TSInteger(TSUInt val) :
        val(val),
        isSigned(false)
    {}

    constexpr TSInteger(TSInt val) :
        val(val),
        isSigned(true)
    {}

    constexpr TSInteger &operator=(TSUInt val)
    {
        this->val = val;
        isSigned = false;

        return *this;
    }

    constexpr TSInteger &operator=(TSInt val)
    {
        this->val = val;
        isSigned = true;

        return *this;
    }

    bool operator==(const TSInteger &num) const;
    bool operator!=(const TSInteger &num) const;
    bool operator<(const TSInteger &num) const;
    bool operator>(const TSInteger &num) const;
    bool operator<=(const TSInteger &num) const;
    bool operator>=(const TSInteger &num) const;
    TSInteger operator+(const TSInteger &num) const;
    TSInteger operator-(const TSInteger &num) const;
    TSInteger operator+() const;
    TSInteger operator-() const;
    TSInteger operator*(const TSInteger &num) const;
    TSInteger operator/(const TSInteger &num) const;
    
    inline TSInteger &operator+=(const TSInteger &num)
    {
        *this = *this + num;
        return *this;
    }
    
    inline TSInteger &operator-=(const TSInteger &num)
    {
        *this = *this - num;
        return *this;
    }
    
    inline TSInteger &operator*=(const TSInteger &num)
    {
        *this = *this * num;
        return *this;
    }
    
    inline TSInteger &operator/=(const TSInteger &num)
    {
        *this = *this / num;
        return *this;
    }
    
    std::string str(bool includeSign = false) const;
private:
    uintmax_t val;
    bool isSigned;
};

constexpr TSInteger operator""_I(TSUInt val)
{
    return val;
}

#endif
