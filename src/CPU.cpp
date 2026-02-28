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

void CPU::updateFlags(bool z, bool n, bool h, bool c) {
    F = ((z << 7) | (n << 6) | (h << 5) | (c << 4)) & 0xF0;
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

void CPU::inst_add(uint8_t operand) {
    int result = A + operand;
    bool z = ((result & 0xFF) == 0);
    bool n = false;
    bool h = ((A & 0x0F) + (operand & 0x0F)) > 0x0F;
    bool c = result > 0xFF;
    A = static_cast<uint8_t>(result);
    updateFlags(z, n, h, c);
}

void CPU::inst_adc(uint8_t operand) {
    uint8_t carry = static_cast<uint8_t>(isFlagSet(Flag::C));
    int result = A + operand + carry;
    bool z = ((result & 0xFF) == 0);
    bool n = false;
    bool h = ((A & 0x0F) + (operand & 0x0F) + carry) > 0x0F;
    bool c = result > 0xFF;
    A = static_cast<uint8_t>(result);
    updateFlags(z, n, h, c);
}

void CPU::inst_sub(uint8_t operand) {
    int result = A - operand;
    bool z = ((result & 0xFF) == 0);
    bool n = true;
    bool h = (A & 0x0F) < (operand & 0x0F);
    bool c = A < operand;
    A = static_cast<uint8_t>(result);
    updateFlags(z, n, h, c);
}

void CPU::inst_sbc(uint8_t operand) {
    uint8_t carry = static_cast<uint8_t>(isFlagSet(Flag::C));
    int result = A - operand - carry;
    bool z = ((result & 0xFF) == 0);
    bool n = true;
    bool h = (A & 0x0F) < ((operand & 0x0F) + carry);
    bool c = A < (static_cast<uint16_t>(operand) + carry);
    A = static_cast<uint8_t>(result);
    updateFlags(z, n, h, c);
}

void CPU::inst_and(uint8_t operand) {
    A &= operand;
    updateFlags(A == 0, false, true, false);
}

void CPU::inst_xor(uint8_t operand) {
    A ^= operand;
    updateFlags(A == 0, false, false, false);
}

void CPU::inst_or(uint8_t operand) {
    A |= operand;
    updateFlags(A == 0, false, false, false);
}

void CPU::inst_cp(uint8_t operand) {
    int result = A - operand;
    bool z = ((result & 0xFF) == 0);
    bool n = true;
    bool h = (A & 0x0F) < (operand & 0x0F);
    bool c = A < operand;
    updateFlags(z, n, h, c);
}

uint8_t CPU::fetch() {
    return bus.read(PC++);
}

void CPU::step() {
    uint8_t opcode = fetch();

    // Block 1:
    if((opcode & 0xC0) == 0x40) {
        if(opcode == 0x76) {
            // TODO Implement halt
            halt();
            uint8_t dest_idx = (opcode >> 3) & 0x07;
            uint8_t src_idx = opcode & 0x07;
            setR8(dest_idx, getR8(src_idx));
        }

    }

    // Block 2:
    if((opcode & 0xC0) == 0x80) {
        uint8_t src_idx = opcode & 0x07;
        uint8_t operand = getR8(src_idx);
        uint8_t operation = (opcode & 0x38) >> 3;
        switch (operation) {
        case 0: inst_add(operand); break;
        case 1: inst_adc(operand); break;
        case 2: inst_sub(operand); break;
        case 3: inst_sbc(operand); break;
        case 4: inst_and(operand); break;
        case 5: inst_xor(operand); break;
        case 6: inst_or(operand);  break;
        case 7: inst_cp(operand);  break;
        default: break;
    }
    }
}