#include "Integer.h"

#include <limits>
#include <cstdint>

using std::experimental::optional;
using std::experimental::nullopt;

constexpr Int Int_MIN = std::numeric_limits<Int>::min();
constexpr Int Int_MAX = std::numeric_limits<Int>::max();
constexpr UInt UInt_MAX = std::numeric_limits<UInt>::max();

bool Integer::operator==(const Integer &num) const {
    if (isSigned && num.isSigned)
        return (Int)val == (Int)num.val;
    else if (!isSigned && num.isSigned)
        return (((UInt)val > Int_MAX) || ((Int)num.val < 0)) ? false : ((UInt)val == (UInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((Int)val < 0) || ((UInt)num.val > Int_MAX)) ? false : ((UInt)val == (UInt)num.val);
    else
        return (UInt)val == (UInt)num.val;
}

bool Integer::operator!=(const Integer &num) const {
    if (isSigned && num.isSigned)
        return (Int)val != (Int)num.val;
    else if (!isSigned && num.isSigned)
        return (((UInt)val > Int_MAX) || ((Int)num.val < 0)) ? true : ((UInt)val != (UInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((Int)val < 0) || ((UInt)num.val > Int_MAX)) ? true : ((UInt)val != (UInt)num.val);
    else
        return (UInt)val != (UInt)num.val;
}

bool Integer::operator<(const Integer &num) const {
    if (isSigned && num.isSigned)
        return (Int)val < (Int)num.val;
    else if (!isSigned && num.isSigned)
        return (((UInt)val > Int_MAX) || ((Int)num.val < 0)) ? false : ((UInt)val < (UInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((Int)val < 0) || ((UInt)num.val > Int_MAX)) ? true : ((UInt)val < (UInt)num.val);
    else
        return (UInt)val < (UInt)num.val;
}

bool Integer::operator>(const Integer &num) const {
    if (isSigned && num.isSigned)
        return (Int)val > (Int)num.val;
    else if (!isSigned && num.isSigned)
        return (((UInt)val > Int_MAX) || ((Int)num.val < 0)) ? true : ((UInt)val > (UInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((Int)val < 0) || ((UInt)num.val > Int_MAX)) ? false : ((UInt)val > (UInt)num.val);
    else
        return (UInt)val > (UInt)num.val;
}

bool Integer::operator<=(const Integer &num) const {
    if (isSigned && num.isSigned)
        return (Int)val <= (Int)num.val;
    else if (!isSigned && num.isSigned)
        return (((UInt)val > Int_MAX) || ((Int)num.val < 0)) ? false : ((UInt)val <= (UInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((Int)val < 0) || ((UInt)num.val > Int_MAX)) ? true : ((UInt)val <= (UInt)num.val);
    else
        return (UInt)val <= (UInt)num.val;
}

bool Integer::operator>=(const Integer &num) const {
    if (isSigned && num.isSigned)
        return (Int)val >= (Int)num.val;
    else if (!isSigned && num.isSigned)
        return (((UInt)val > Int_MAX) || ((Int)num.val < 0)) ? true : ((UInt)val >= (UInt)num.val);
    else if (isSigned && !num.isSigned)
        return (((Int)val < 0) || ((UInt)num.val > Int_MAX)) ? false : ((UInt)val >= (UInt)num.val);
    else
        return (UInt)val >= (UInt)num.val;
}

Integer Integer::operator+(const Integer &num) const {
    if (isSigned && ((Int)val < 0)) {
        if (num.isSigned && ((Int)num.val < 0)) {
            if ((Int_MIN - (Int)num.val) > (Int)val)
                throw std::overflow_error("overflow (too low)");

            return (Int)val + (Int)num.val;
        }
        else if (!num.isSigned && ((UInt)num.val > Int_MAX))
            return (UInt)num.val - (UInt)(-(Int)val);
        else
            return (Int)val + (Int)num.val;
    } else if (!isSigned && ((UInt)val > Int_MAX)) {
        if (num.isSigned && ((Int)num.val < 0))
            return (UInt)val - (UInt)(-(Int)num.val);
        else if (!num.isSigned && ((UInt)num.val > Int_MAX))
            throw std::overflow_error("overflow (too high)");
        else {
            if ((UInt_MAX - (UInt)num.val) < (UInt)val)
                throw std::overflow_error("overflow (too high)");

            return (UInt)val + (UInt)num.val;
        }
    } else {
        if (num.isSigned && ((Int)num.val < 0))
            return (Int)val + (Int)num.val;
        else if (!num.isSigned && ((UInt)num.val > Int_MAX)) {
            if ((UInt_MAX - (UInt)num.val) < (UInt)val)
                throw std::overflow_error("overflow (too high)");

            return (UInt)val + (UInt)num.val;
        } else
            return (UInt)val + (UInt)num.val;
    }
}

Integer Integer::operator-(const Integer &num) const {
    if (isSigned && ((Int)val < 0)) {
        if (num.isSigned && ((Int)num.val < 0))
            return (Int)val - (Int)num.val;
        else if (!num.isSigned && ((UInt)num.val > Int_MAX))
            throw std::overflow_error("overflow (too low)");
        else {
            if ((Int_MIN + (Int)num.val) > (Int)val)
                throw std::overflow_error("overflow (too low)");

            return (Int)val - (Int)num.val;
        }
    } else if (!isSigned && ((UInt)val > Int_MAX)) {
        if (num.isSigned && ((Int)num.val < 0)) {
            if ((UInt_MAX - (UInt)(-(Int)num.val)) < (UInt)val)
                throw std::overflow_error("overflow (too high)");

            return (UInt)val + (UInt)(-(Int)num.val);
        } else if (!num.isSigned && ((UInt)num.val > Int_MAX)) {
            if ((UInt)val > (UInt)num.val)
                return (Int)((UInt)val - (UInt)num.val);
            else
                return -(Int)((UInt)num.val - (UInt)val);
        } else
            return (UInt)val - (UInt)num.val;
    } else {
        if (num.isSigned && ((Int)num.val < 0))
            return (UInt)val + (UInt)(-(Int)num.val);
        else if (!num.isSigned && ((UInt)num.val > Int_MAX)) {
            if ((UInt)(-Int_MIN) < ((UInt)num.val - (UInt)val))
                throw std::overflow_error("overflow (too low)");

            return -(Int)((UInt)num.val - (UInt)val);
        }
        else
            return (Int)val - (Int)num.val;
    }
}

Integer Integer::operator+() const {
    return *this;
}

Integer Integer::operator-() const {
    if (!isSigned && ((UInt)val > (UInt)(-Int_MIN)))
        throw std::overflow_error("overflow (too big to negate)");

    return -(Int)val;
}

Integer Integer::operator*(const Integer &num) const {
    bool isResultNegative = (isSigned && ((Int)val < 0)) != (num.isSigned && ((Int)num.val < 0));

    UInt mult1 = isSigned ? (UInt)llabs((Int)val) : (UInt)val;
    UInt mult2 = num.isSigned ? (UInt)llabs((Int)num.val) : (UInt)num.val;

    UInt res = mult1 * mult2;

    if ((mult1 != 0) && ((res / mult1) != mult2))
        throw std::overflow_error("overflow (too large multipliers)");

    if (isResultNegative) {
        if (res > (UInt)(-Int_MIN))
            throw std::overflow_error("overflow (too large multipliers)");

        return -(Int)res;
    } else
        return (UInt)res;
}

Integer Integer::operator/(const Integer &num) const {
    bool isResultNegative = (isSigned && ((Int)val < 0)) != (num.isSigned && ((Int)num.val < 0));

    UInt div1 = isSigned ? (UInt)llabs((Int)val) : (UInt)val;
    UInt div2 = num.isSigned ? (UInt)llabs((Int)num.val) : (UInt)num.val;

    if (div2 == 0)
        throw std::overflow_error("overflow (division by zero)");

    UInt res = div1 / div2;

    if (isResultNegative) {
        if (res > (UInt)(-Int_MIN))
            throw std::overflow_error("overflow (too large divident)");

        return -(Int)res;
    } else
        return (UInt)res;
}

std::string Integer::str(bool includeSign) const {
    std::string str =  isSigned ? std::to_string((Int)val) : std::to_string((UInt)val);

    if (includeSign && (*this > 0))
        str = "+" + str;

    return str;
}

optional<Integer::Size> Integer::sizeSigned() const {
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

optional<Integer::Size> Integer::sizeUnsigned() const {
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

Integer::Size Integer::sizeAny() const {
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

vector<uchar> Integer::getCharArraySigned(Size size) const {
    vector<uchar> res;

    if (size == Size::S_8) {
        int8_t vecVal = (int8_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int8_t); ++i)
            res.push_back(p[i]);
    } else if (size == Size::S_16) {
        int16_t vecVal = (int16_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int16_t); ++i)
            res.push_back(p[i]);
    } else if (size == Size::S_32) {
        int32_t vecVal = (int32_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int32_t); ++i)
            res.push_back(p[i]);
    } else {
        int64_t vecVal = (int64_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(int64_t); ++i)
            res.push_back(p[i]);
    }

    return res;
}

vector<uchar> Integer::getCharArrayUnsigned(Size size) const {
    vector<uchar> res;

    if (size == Size::S_8) {
        uint8_t vecVal = (uint8_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint8_t); ++i)
            res.push_back(p[i]);
    } else if (size == Size::S_16) {
        uint16_t vecVal = (uint16_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint16_t); ++i)
            res.push_back(p[i]);
    } else if (size == Size::S_32) {
        uint32_t vecVal = (uint32_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint32_t); ++i)
            res.push_back(p[i]);
    } else {
        uint64_t vecVal = (uint64_t)val;
        uchar *p = (uchar *)&vecVal;

        for (size_t i = 0; i < sizeof(uint64_t); ++i)
            res.push_back(p[i]);
    }

    return res;
}

vector<uchar> Integer::getCharArrayAny(Size size) const {
    return getCharArrayUnsigned(size);
}

Integer Integer::getMaxValSigned(Size size) {
    switch (size) {
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

Integer Integer::getMaxValUnsigned(Size size) {
    switch (size) {
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

Integer Integer::getMaxValAny(Size size) {
    return getMaxValUnsigned(size);
}

Integer::Size Integer::nextSize(Size size) {
    switch (size) {
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
