#include "CPU.h"

CPU::CPU(Bus& b) : bus{b} {}


// AF Pair
uint16_t CPU::getAF() const { return (static_cast<uint16_t>(A) << 8) | F; }
void CPU::setAF(uint16_t value) {A = (value >> 8) & 0xFF; F = value & 0xF0; }

// BC Pair
uint16_t CPU::getBC() const { return (static_cast<uint16_t>(B) << 8) | C; }
void CPU::setBC(uint16_t value) {B = (value >> 8) & 0xFF; C = value & 0xFF; }

// DE Pair
uint16_t CPU::getDE() const { return (static_cast<uint16_t>(D) << 8) | E; }
void CPU::setDE(uint16_t value) {D = (value >> 8) & 0xFF; E = value & 0xFF; }

// HL Pair
uint16_t CPU::getHL() const { return (static_cast<uint16_t>(H) << 8) | L; }
void CPU::setHL(uint16_t value) {H = (value >> 8) & 0xFF; L = value & 0xFF; }

uint16_t CPU::getSP() const { return SP; }
void CPU::setSP(uint16_t value) { SP = value; }

uint16_t CPU::getPC() const { return PC; }
void CPU::setPC(uint16_t value) { PC = value; }

bool CPU::isFlagSet(Flag flag) const {
    return (F & static_cast<uint8_t>(flag)) != 0;
}

uint8_t CPU::getR8(uint8_t reg_index) const {
    switch (static_cast<Reg8>(reg_index))
    {
    case Reg8::B: return B;
    case Reg8::C: return C;
    case Reg8::D: return D;
    case Reg8::E: return E;
    case Reg8::H: return H;
    case Reg8::L: return L;
    case Reg8::HL_MEM: return bus.read(getHL());
    case Reg8::A: return A;
    default: return 0;
    }
}

void CPU::setR8(uint8_t reg_index, uint8_t value) {
    switch (static_cast<Reg8>(reg_index))
    {
    case Reg8::B:
        B = value;
        break;
    case Reg8::C:
        C = value;
        break;
    case Reg8::D:
        D = value;
        break;
    case Reg8::E:
        E = value;
        break;
    case Reg8::H:
        H =value ;
        break;
    case Reg8::L:
        L = value;
        break;
    case Reg8::HL_MEM:
        bus.write(getHL(), value);
        break;
    case Reg8::A:
        A = value;
        break;
    default:
        break;
    }
}

uint16_t CPU::getR16(uint8_t reg_index) const {
    switch (static_cast<Reg16>(reg_index))
    {
    case Reg16::BC: return getBC();
    case Reg16::DE: return getDE();
    case Reg16::HL: return getHL();
    case Reg16::SP: return getSP();
    default: return 0;
    }
}

void CPU::setR16(uint8_t reg_index, uint16_t value) {
    switch (static_cast<Reg16>(reg_index))
    {
    case Reg16::BC:
        setBC(value);
        break;
    case Reg16::DE:
        setDE(value);
        break;
    case Reg16::HL:
        setHL(value);
        break;
    case Reg16::SP:
        setSP(value);
        break;
    default:
        break;
    }
}

uint16_t CPU::getR16stk(uint8_t reg_index) const {
    switch (static_cast<Reg16Stk>(reg_index))
    {
    case Reg16Stk::BC: return getBC();
    case Reg16Stk::DE: return getDE();
    case Reg16Stk::HL: return getHL();
    case Reg16Stk::AF: return getAF();
    default: return 0;
    }
}

void CPU::setR16stk(uint8_t reg_index, uint16_t value) {
    switch (static_cast<Reg16Stk>(reg_index))
    {
    case Reg16Stk::BC:
        setBC(value);
        break;
    case Reg16Stk::DE:
        setDE(value);
        break;
    case Reg16Stk::HL:
        setHL(value);
        break;
    case Reg16Stk::AF:
        setAF(value);
        break;
    default:
        break;
    }
}
uint16_t CPU::getR16mem(uint8_t reg_index) {
    uint16_t HL = getHL();
    switch (reg_index)
    {
    case 0: return getBC();
    case 1: return getDE();
    case 2: setHL(HL + 1); return HL;
    case 3: setHL(HL - 1); return HL;
    default: return 0;
    }
}

bool CPU::checkCond(uint8_t index) const{
    switch (static_cast<Cond>(index))
    {
    case Cond::NZ: return !isFlagSet(Flag::Z);
    case Cond::Z: return isFlagSet(Flag::Z);
    case Cond::NC: return !isFlagSet(Flag::C);
    case Cond::C: return isFlagSet(Flag::C);
    default: return 0;
    }
}

