#pragma once
#include <cstdint>
#include "Bus.h"

class CPU
{
private:
    enum class Flag : uint8_t { Z = 0x80, N = 0x40, H = 0x20, C = 0x10};
    enum class Reg8 : uint8_t { B = 0, C = 1, D = 2, E = 3, H = 4, L = 5, HL_MEM = 6, A = 7 };
    enum class Reg16 : uint8_t { BC = 0, DE = 1, HL = 2, SP = 3 };
    enum class Reg16Stk : uint8_t { BC = 0, DE = 1, HL = 2, AF = 3 };
    enum class Cond : uint8_t { NZ = 0, Z = 1, NC = 2, C = 3};
    // Registers
    uint8_t A, F;
    uint8_t B, C;
    uint8_t D, E;
    uint8_t H, L;
    uint16_t SP, PC;

    // 16-bit Register Pairs
    uint16_t getAF() const;
    void setAF(uint16_t value);
    uint16_t getBC() const;
    void setBC(uint16_t value);
    uint16_t getDE() const;
    void setDE(uint16_t value);
    uint16_t getHL() const;
    void setHL(uint16_t value);
    uint16_t getSP() const;
    void setSP(uint16_t value);
    uint16_t getPC() const;
    void setPC(uint16_t value);
    uint8_t getR8(uint8_t reg_index) const;
    void setR8(uint8_t reg_index, uint8_t value);
    bool isFlagSet(Flag flag) const;
    void updateFlags(bool z, bool n, bool h, bool c);

    // Opcode Table Helpers
    uint16_t getR16(uint8_t reg_index) const;
    void setR16(uint8_t reg_index, uint16_t value);
    uint16_t getR16stk(uint8_t reg_index) const;
    void setR16stk(uint8_t reg_index, uint16_t value);
    uint16_t getR16mem(uint8_t reg_index);
    bool checkCond(uint8_t index) const;

    // ALU Helpers
    void inst_add(uint8_t operand);
    void inst_adc(uint8_t operand);
    void inst_sub(uint8_t operand);
    void inst_sbc(uint8_t operand);
    void inst_and(uint8_t operand);
    void inst_xor(uint8_t operand);
    void inst_or(uint8_t operand);
    void inst_cp(uint8_t operand);

    // CPU ISA
    uint8_t fetch();
    void step();
    void halt();
    // For memory access;
    Bus& bus;
public:
    CPU(Bus& b);
};


