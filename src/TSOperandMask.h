#ifndef _TSOPERANDMASK_H_
#define _TSOPERANDMASK_H_

#include <bitset>

namespace TSOperandMask
{

enum : unsigned long long
{
    UREG               = 1ull << 0,
    MEM                = 1ull << 1,
    IMM                = 1ull << 2,

    S8                 = 1ull << 16,
    S16                = 1ull << 17,
    S32                = 1ull << 18,
    S64                = 1ull << 19,
    S_ANY              = S8 | S16 | S32 | S64,

    SREG               = S16 | (1ull << 3),

    UREG8              = UREG | S8,
    UREG16             = UREG | S16,
    UREG32             = UREG | S32,
    UREG64             = UREG | S64,

    MEM_16             = MEM | (1ull << 24),
    MEM_32             = MEM | (1ull << 25),
    MEM_MODE_ANY       = MEM_16 | MEM_32,

    MEM_BASE           = 1ull << 28,
    MEM_32_INDEX       = MEM_32 | (1ull << 29),
    MEM_32_INDEX_MULT1 = MEM_32_INDEX | (1ull << 30),
    MEM_32_INDEX_MULT2 = MEM_32_INDEX | (1ull << 31),
    MEM_32_INDEX_MULT4 = MEM_32_INDEX | (1ull << 32),
    MEM_32_INDEX_MULT8 = MEM_32_INDEX | (1ull << 33),

    OPVAL0             = 1ull << 48,
    OPVAL1             = 1ull << 49,
    OPVAL2             = 1ull << 50,
    OPVAL3             = 1ull << 51,
    OPVAL4             = 1ull << 52,
    OPVAL5             = 1ull << 53,
    OPVAL6             = 1ull << 54,
    OPVAL7             = 1ull << 55,
    OPVAL_ANY          = OPVAL0 | OPVAL1 | OPVAL2 | OPVAL3 | OPVAL4 | OPVAL5 | OPVAL6 | OPVAL7,

    AL                 = UREG8 | OPVAL0,
    CL                 = UREG8 | OPVAL1,
    DL                 = UREG8 | OPVAL2,
    BL                 = UREG8 | OPVAL3,
    AH                 = UREG8 | OPVAL4,
    CH                 = UREG8 | OPVAL5,
    DH                 = UREG8 | OPVAL6,
    BH                 = UREG8 | OPVAL7,

    AX                 = UREG16 | OPVAL0,
    CX                 = UREG16 | OPVAL1,
    DX                 = UREG16 | OPVAL2,
    BX                 = UREG16 | OPVAL3,
    SP                 = UREG16 | OPVAL4,
    BP                 = UREG16 | OPVAL5,
    SI                 = UREG16 | OPVAL6,
    DI                 = UREG16 | OPVAL7,

    EAX                = UREG32 | OPVAL0,
    ECX                = UREG32 | OPVAL1,
    EDX                = UREG32 | OPVAL2,
    EBX                = UREG32 | OPVAL3,
    ESP                = UREG32 | OPVAL4,
    EBP                = UREG32 | OPVAL5,
    ESI                = UREG32 | OPVAL6,
    EDI                = UREG32 | OPVAL7,

    UREG8_ANY          = UREG8 | OPVAL_ANY,
    UREG16_ANY         = UREG16 | OPVAL_ANY,
    UREG32_ANY         = UREG32 | OPVAL_ANY,
    UREG64_ANY         = UREG64 | OPVAL_ANY,
    UREG_ANY           = UREG8_ANY | UREG16_ANY | UREG32_ANY | UREG64_ANY,

    ES                 = SREG | OPVAL0,
    CS                 = SREG | OPVAL1,
    SS                 = SREG | OPVAL2,
    DS                 = SREG | OPVAL3,
    FS                 = SREG | OPVAL4,
    GS                 = SREG | OPVAL5,

    SREG_ANY           = SREG | OPVAL_ANY,

    MEM_16_BX_SI       = MEM_16 | MEM_BASE | OPVAL0,
    MEM_16_BX_DI       = MEM_16 | MEM_BASE | OPVAL1,
    MEM_16_BP_SI       = MEM_16 | MEM_BASE | OPVAL2,
    MEM_16_BP_DI       = MEM_16 | MEM_BASE | OPVAL3,
    MEM_16_SI          = MEM_16 | MEM_BASE | OPVAL4,
    MEM_16_DI          = MEM_16 | MEM_BASE | OPVAL5,
    MEM_16_BP          = MEM_16 | MEM_BASE | OPVAL6,
    MEM_16_BX          = MEM_16 | MEM_BASE | OPVAL7,

    MEM_32_BASE_EAX    = MEM_32 | MEM_BASE | OPVAL0,
    MEM_32_BASE_ECX    = MEM_32 | MEM_BASE | OPVAL1,
    MEM_32_BASE_EDX    = MEM_32 | MEM_BASE | OPVAL2,
    MEM_32_BASE_EBX    = MEM_32 | MEM_BASE | OPVAL3,
    MEM_32_BASE_ESP    = MEM_32 | MEM_BASE | OPVAL4,
    MEM_32_BASE_EBP    = MEM_32 | MEM_BASE | OPVAL5,
    MEM_32_BASE_ESI    = MEM_32 | MEM_BASE | OPVAL6,
    MEM_32_BASE_EDI    = MEM_32 | MEM_BASE | OPVAL7,

    MEM_32_INDEX_EAX   = MEM_32_INDEX | (OPVAL0 << 8),
    MEM_32_INDEX_ECX   = MEM_32_INDEX | (OPVAL1 << 8),
    MEM_32_INDEX_EDX   = MEM_32_INDEX | (OPVAL2 << 8),
    MEM_32_INDEX_EBX   = MEM_32_INDEX | (OPVAL3 << 8),
    MEM_32_INDEX_ESP   = MEM_32_INDEX | (OPVAL4 << 8),
    MEM_32_INDEX_EBP   = MEM_32_INDEX | (OPVAL5 << 8),
    MEM_32_INDEX_ESI   = MEM_32_INDEX | (OPVAL6 << 8),
    MEM_32_INDEX_EDI   = MEM_32_INDEX | (OPVAL7 << 8)
};

class Mask : public std::bitset<64>
{
public:
    constexpr Mask() noexcept :
        std::bitset<64>()
    {}

    constexpr Mask(unsigned long long val) noexcept :
        std::bitset<64>(val)
    {}

    constexpr Mask(const std::bitset<64> &bitSet) :
        std::bitset<64>(bitSet)
    {}

    inline Mask &operator&=(const Mask &mask) noexcept
    {
        std::bitset<64>::operator&=(mask);

        return *this;
    }

    inline Mask &operator|=(const Mask &mask) noexcept
    {
        std::bitset<64>::operator|=(mask);

        return *this;
    }

    inline Mask &operator^=(const Mask &mask) noexcept
    {
        std::bitset<64>::operator^=(mask);

        return *this;
    }

    inline Mask &operator<<=(size_t pos) noexcept
    {
        std::bitset<64>::operator<<=(pos);

        return *this;
    }

    inline Mask &operator>>=(size_t pos) noexcept
    {
        std::bitset<64>::operator>>=(pos);

        return *this;
    }

    inline Mask operator~() const noexcept
    {
        return std::bitset<64>::operator~();
    }

    inline Mask operator<<(size_t pos) const noexcept
    {
        return std::bitset<64>::operator<<(pos);
    }

    inline Mask operator>>(size_t pos) const noexcept
    {
        return std::bitset<64>::operator>>(pos);
    }

    inline bool operator==(const Mask &mask) const noexcept
    {
        return std::bitset<64>::operator==(mask);
    }

    inline bool operator!=(const Mask &mask) const noexcept
    {
        return std::bitset<64>::operator!=(mask);
    }

    inline bool match(Mask mask) const
    {
        return !((*this & mask) ^ mask).any();
    }

    inline bool matchAny(Mask mask) const
    {
        return (*this & mask).any();
    }
};

inline Mask operator&(const Mask &mask1, const Mask &mask2)
{
    return std::bitset<64>(mask1) & std::bitset<64>(mask2);
}

inline Mask operator|(const Mask &mask1, const Mask &mask2)
{
    return std::bitset<64>(mask1) | std::bitset<64>(mask2);
}

inline Mask operator^(const Mask &mask1, const Mask &mask2)
{
    return std::bitset<64>(mask1) ^ std::bitset<64>(mask2);
}

}

#endif
