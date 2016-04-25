#ifndef _TSINTEGER_H_
#define _TSINTEGER_H_

#include <stdexcept>
#include <string>
#include <experimental/optional>
#include <vector>

typedef long long TSInt;
typedef unsigned long long TSUInt;

class TSInteger {
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

    constexpr TSInteger(int val) :
        TSInteger((TSInt)val)
    {}

    constexpr TSInteger(long val) :
        TSInteger((TSInt)val)
    {}

    constexpr TSInteger(unsigned int val) :
        TSInteger((TSUInt)val)
    {}

    constexpr TSInteger(unsigned long val) :
        TSInteger((TSUInt)val)
    {}

    constexpr TSInteger &operator=(TSUInt val) {
        this->val = val;
        isSigned = false;

        return *this;
    }

    constexpr TSInteger &operator=(TSInt val) {
        this->val = val;
        isSigned = true;

        return *this;
    }

    constexpr TSInteger &operator=(int val) {
        return operator=((TSInt)val);
    }

    constexpr TSInteger &operator=(long val) {
        return operator=((TSInt)val);
    }

    constexpr TSInteger &operator=(unsigned int val) {
        return operator=((TSUInt)val);
    }

    constexpr TSInteger &operator=(unsigned long val) {
        return operator=((TSUInt)val);
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
    
    inline TSInteger &operator+=(const TSInteger &num) {
        *this = *this + num;
        return *this;
    }
    
    inline TSInteger &operator-=(const TSInteger &num) {
        *this = *this - num;
        return *this;
    }
    
    inline TSInteger &operator*=(const TSInteger &num) {
        *this = *this * num;
        return *this;
    }
    
    inline TSInteger &operator/=(const TSInteger &num) {
        *this = *this / num;
        return *this;
    }
    
    std::string str(bool includeSign = false) const;

    enum class Size {
        S_8,
        S_16,
        S_32,
        S_64
    };

    std::experimental::optional<Size> sizeSigned() const;
    std::experimental::optional<Size> sizeUnsigned() const;
    Size sizeAny() const;
    std::vector<unsigned char> getCharArraySigned(Size size) const;
    std::vector<unsigned char> getCharArrayUnsigned(Size size) const;
    std::vector<unsigned char> getCharArrayAny(Size size) const;

    static TSInteger getMaxValSigned(Size size);
    static TSInteger getMaxValUnsigned(Size size);
    static TSInteger getMaxValAny(Size size);
    static Size nextSize(Size size);
private:
    TSUInt val;
    bool isSigned;
};

constexpr TSInteger operator""_I(TSUInt val) {
    return val;
}

#endif
