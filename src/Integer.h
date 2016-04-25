#ifndef _INTEGER_H_
#define _INTEGER_H_

#include <stdexcept>
#include <string>
#include <experimental/optional>
#include <vector>

typedef long long Int;
typedef unsigned long long UInt;

class Integer {
public:
    constexpr Integer() :
        Integer((UInt)0)
    {}

    constexpr Integer(UInt val) :
        val(val),
        isSigned(false)
    {}

    constexpr Integer(Int val) :
        val(val),
        isSigned(true)
    {}

    constexpr Integer(int val) :
        Integer((Int)val)
    {}

    constexpr Integer(long val) :
        Integer((Int)val)
    {}

    constexpr Integer(unsigned int val) :
        Integer((UInt)val)
    {}

    constexpr Integer(unsigned long val) :
        Integer((UInt)val)
    {}

    constexpr Integer &operator=(UInt val) {
        this->val = val;
        isSigned = false;

        return *this;
    }

    constexpr Integer &operator=(Int val) {
        this->val = val;
        isSigned = true;

        return *this;
    }

    constexpr Integer &operator=(int val) {
        return operator=((Int)val);
    }

    constexpr Integer &operator=(long val) {
        return operator=((Int)val);
    }

    constexpr Integer &operator=(unsigned int val) {
        return operator=((UInt)val);
    }

    constexpr Integer &operator=(unsigned long val) {
        return operator=((UInt)val);
    }

    bool operator==(const Integer &num) const;
    bool operator!=(const Integer &num) const;
    bool operator<(const Integer &num) const;
    bool operator>(const Integer &num) const;
    bool operator<=(const Integer &num) const;
    bool operator>=(const Integer &num) const;
    Integer operator+(const Integer &num) const;
    Integer operator-(const Integer &num) const;
    Integer operator+() const;
    Integer operator-() const;
    Integer operator*(const Integer &num) const;
    Integer operator/(const Integer &num) const;
    
    inline Integer &operator+=(const Integer &num) {
        *this = *this + num;
        return *this;
    }
    
    inline Integer &operator-=(const Integer &num) {
        *this = *this - num;
        return *this;
    }
    
    inline Integer &operator*=(const Integer &num) {
        *this = *this * num;
        return *this;
    }
    
    inline Integer &operator/=(const Integer &num) {
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

    static Integer getMaxValSigned(Size size);
    static Integer getMaxValUnsigned(Size size);
    static Integer getMaxValAny(Size size);
    static Size nextSize(Size size);
private:
    UInt val;
    bool isSigned;
};

constexpr Integer operator""_I(UInt val) {
    return val;
}

#endif
