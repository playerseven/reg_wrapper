#ifndef REG_WRAPPER_H
#define REG_WRAPPER_H

#include <cstdint>
#include <limits>

namespace rw {
    
template<uint32_t Addr, uint8_t RegSize = 32>    
class RegWrapper {
    static_assert(RegSize <= 32, "Register size could not be more than 32 bits");
public:
    using Reg = volatile uint32_t;

    RegWrapper() = default;
    RegWrapper(const RegWrapper&) = default;
    RegWrapper& operator=(const RegWrapper&) = default;
    
    RegWrapper& operator=(uint32_t val) noexcept {
        *ptr = val;
        return *this;
    };

    Reg& get() noexcept { return *ptr; }
    const Reg& get() const noexcept { return *ptr; }
    
    operator Reg& () noexcept { return *ptr; }  
    operator const Reg& () const noexcept { return *ptr; }  
    
    template<int Bit>
    static constexpr void checkBitNumber() noexcept {
        static_assert(Bit >= 0 && Bit < RegSize, "Wrong bit value");
    }
    
    template<int FirstBit, int LastBit>
    static constexpr void checkBitRange() noexcept {
        checkBitNumber<FirstBit>();
        checkBitNumber<LastBit>();
        static_assert(FirstBit <= LastBit, "First bit should be less than last bit");
    }
    
    template<int FirstBit, int LastBit>
    static constexpr uint32_t convValToBitRangeVal(uint32_t val) noexcept {
        checkBitRange<FirstBit, LastBit>();
        return (val & ((1ull << (LastBit - FirstBit + 1)) - 1)) << FirstBit;
    }
    
    template<int FirstBit, int LastBit>
    static constexpr uint32_t convBitRangeValToVal(uint32_t bit_range) noexcept {
        checkBitRange<FirstBit, LastBit>();
        return (bit_range >> FirstBit) & ((1ull << (LastBit - FirstBit + 1)) - 1);
    }
    
    template<int Bit>
    bool getBit() const noexcept { 
        checkBitNumber<Bit>();
        return *ptr & (1 << Bit); 
    }
    
    template<int Bit>
    void setBit() const noexcept { 
        checkBitNumber<Bit>();
        *ptr |= (1 << Bit); 
    }
    
    template<int Bit>
    void resetBit() const noexcept { 
        checkBitNumber<Bit>();
        *ptr &= ~(1 << Bit); 
    }
    
    template<int FirstBit, int LastBit>
    uint32_t getBitRangeVal() const noexcept {
        checkBitRange<FirstBit, LastBit>();
        return convBitRangeValToVal<FirstBit, LastBit>(*ptr);
    }
    
    template<int FirstBit, int LastBit>
    void setBitRangeVal(uint32_t val) noexcept {
        checkBitRange<FirstBit, LastBit>();
        uint32_t reg = *ptr;
        reg &= ~convValToBitRangeVal<FirstBit, LastBit>(std::numeric_limits<uint32_t>::max());
        reg |= convValToBitRangeVal<FirstBit, LastBit>(val);
        *ptr = reg;
    }
    
    template<int FirstBit, int LastBit>
    void setBitRange() noexcept {
        checkBitRange<FirstBit, LastBit>();
        *ptr |= convValToBitRangeVal<FirstBit, LastBit>(std::numeric_limits<uint32_t>::max());
    }
    
    template<int FirstBit, int LastBit>
    void resetBitRange() noexcept {
        checkBitRange<FirstBit, LastBit>();
        *ptr &= ~convValToBitRangeVal<FirstBit, LastBit>(std::numeric_limits<uint32_t>::max());
    }
    
private:
    Reg* const ptr{reinterpret_cast<Reg*>(Addr)};
};

static_assert(RegWrapper<0>::convValToBitRangeVal<0, 31>(std::numeric_limits<uint32_t>::max()) == 
              std::numeric_limits<uint32_t>::max(), "Incorrect behavior of convValToBitRangeVal");
static_assert(RegWrapper<0>::convValToBitRangeVal<0, 31>(std::numeric_limits<uint32_t>::min()) == 
              std::numeric_limits<uint32_t>::min(), "Incorrect behavior of convValToBitRangeVal");
static_assert(RegWrapper<0>::convValToBitRangeVal<30, 31>(0x3) == 
              (0x3 << 30), "Incorrect behavior of convValToBitRangeVal");
static_assert(RegWrapper<0>::convValToBitRangeVal<31, 31>(0x3) == 
              (1 << 31), "Incorrect behavior of convValToBitRangeVal");

static_assert(RegWrapper<0>::convBitRangeValToVal<0, 31>(std::numeric_limits<uint32_t>::max()) == 
              std::numeric_limits<uint32_t>::max(), "Incorrect behavior of convBitRangeValToVal");
static_assert(RegWrapper<0>::convBitRangeValToVal<0, 31>(std::numeric_limits<uint32_t>::min()) == 
              std::numeric_limits<uint32_t>::min(), "Incorrect behavior of convBitRangeValToVal");
static_assert(RegWrapper<0>::convBitRangeValToVal<30, 31>(0x3 << 30) == 
              0x3, "Incorrect behavior of convBitRangeValToVal");
static_assert(RegWrapper<0>::convBitRangeValToVal<31, 31>(1 << 31) == 
              1, "Incorrect behavior of convBitRangeValToVal");

}

#endif
