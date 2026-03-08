#include "CPU.h"

CPU::CPU(Bus& b) : A{0x01}, F{0x80}, B{0x00}, C{0x13}, D{0x00}, E{0xC1}, H{0x84}, L{0x03}, PC{0x0100}, SP{0xFFFE}, bus{b}, isHalted{false} {}


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
    case Reg16::SP: return SP;
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
        SP = value;
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
    uint32_t result = A + operand;
    bool z = ((result & 0xFF) == 0);
    bool n = false;
    bool h = ((A & 0x0F) + (operand & 0x0F)) > 0x0F;
    bool c = result > 0xFF;
    A = static_cast<uint8_t>(result);
    updateFlags(z, n, h, c);
}

void CPU::inst_adc(uint8_t operand) {
    uint8_t carry = static_cast<uint8_t>(isFlagSet(Flag::C));
    uint32_t result = A + operand + carry;
    bool z = ((result & 0xFF) == 0);
    bool n = false;
    bool h = ((A & 0x0F) + (operand & 0x0F) + carry) > 0x0F;
    bool c = result > 0xFF;
    A = static_cast<uint8_t>(result);
    updateFlags(z, n, h, c);
}

void CPU::inst_sub(uint8_t operand) {
    uint32_t result = A - operand;
    bool z = ((result & 0xFF) == 0);
    bool n = true;
    bool h = (A & 0x0F) < (operand & 0x0F);
    bool c = A < operand;
    A = static_cast<uint8_t>(result);
    updateFlags(z, n, h, c);
}

void CPU::inst_sbc(uint8_t operand) {
    uint8_t carry = static_cast<uint8_t>(isFlagSet(Flag::C));
    uint32_t result = A - operand - carry;
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
    uint32_t result = A - operand;
    bool z = ((result & 0xFF) == 0);
    bool n = true;
    bool h = (A & 0x0F) < (operand & 0x0F);
    bool c = A < operand;
    updateFlags(z, n, h, c);
}

void CPU::pushStack(uint16_t value) {

    bus.write(--SP,(value >> 8 & 0xFF));
    bus.write(--SP, value & 0xFF);
}

uint16_t CPU::popStack() {
    uint8_t low = bus.read(SP++);
    uint8_t high = bus.read(SP++);
    return ((static_cast<uint16_t>(high) << 8) | low);
}

uint8_t CPU::inst_rlc(uint8_t operand, bool is_cb_prefix) {
    uint8_t msb = operand >> 7;
    uint8_t result = (operand << 1) | msb;
    bool z = is_cb_prefix ? (result == 0) : false;
    updateFlags(z, false, false, msb == 1);
    return result;
}

uint8_t CPU::inst_rrc(uint8_t operand, bool is_cb_prefix) {
    uint8_t lsb = operand & 0x01;
    uint8_t result = (operand >> 1) | (lsb << 7);
    bool z = is_cb_prefix ? (result == 0) : false;
    updateFlags(z, false, false, lsb == 1);
    return result;
}

uint8_t CPU::inst_rl(uint8_t operand, bool is_cb_prefix) {
    uint8_t msb = operand >> 7;
    uint8_t carry = isFlagSet(Flag::C) ? 1 : 0;
    uint8_t result = (operand << 1) | carry;
    bool z = is_cb_prefix ? (result == 0) : false;
    updateFlags(z, false, false, msb == 1);
    return result;
}

uint8_t CPU::inst_rr(uint8_t operand, bool is_cb_prefix) {
    uint8_t lsb = operand & 0x01;
    uint8_t carry = isFlagSet(Flag::C) ? 1 : 0;
    uint8_t result = (operand >> 1) | (carry << 7);
    bool z = is_cb_prefix ? (result == 0) : false;
    updateFlags(z, false, false, lsb == 1);
    return result;
}

uint8_t CPU::fetch8() {
    return bus.read(PC++);
}

uint16_t CPU::fetch16() {
    uint8_t low = fetch8();
    uint8_t high = fetch8();
    return ((static_cast<uint16_t>(high) << 8) | low);
}

void CPU::step() {
    
    // Interrupt Handler
    uint8_t IF = bus.read(0xFF0F);
    uint8_t IE = bus.read(0xFFFF);

    uint8_t pending = (IF & IE) & 0x1F;
    if(pending > 0) {
        isHalted = false;

        if (IME) {
            // VBlank
            if(pending & 0x01) {
                bus.write(0xFF0F, IF & 0xFE);
                IME = false;
                pushStack(PC);
                PC = 0x40;
                return;
            } 
            // LCD
            else if(pending & 0x02) {
                bus.write(0xFF0F, IF & 0xFD);
                IME = false;
                pushStack(PC);
                PC = 0x48;
                return;            
            } 
            // Timer
            else if(pending & 0x04) {
                bus.write(0xFF0F, IF & 0xFB);
                IME = false;
                pushStack(PC);
                PC = 0x50; 
                return;          
            } 
            // Serial
            else if(pending & 0x08) {
                bus.write(0xFF0F, IF & 0xF7);
                IME = false;
                pushStack(PC);
                PC = 0x58; 
                return;          
            } 
            // Joypad
            else if(pending & 0x10) {
                bus.write(0xFF0F, IF & 0xEF);
                IME = false;
                pushStack(PC);
                PC = 0x60; 
                return;          
            } 
        }

    }

    // This uses a for when step starts returning cycles
    if(isHalted)
        return;

    uint8_t opcode = fetch8();

    // Block 0:
    if((opcode & 0xC0) == 0x00) {
        //ld
        if((opcode & 0xCF) == 0x01) {
            uint8_t dest_idx = (opcode >> 4) & 0x03;
            uint16_t imm16 = fetch16(); 
            setR16(dest_idx, imm16);
            return;
        }
        if((opcode & 0xCF) == 0x02) {
            uint8_t dest_idx = (opcode >> 4) & 0x03;
            bus.write(getR16mem(dest_idx), A);
            return;
        }
        if((opcode & 0xCF) == 0x0A) {
            uint8_t src_idx = (opcode >> 4) & 0x03;
            A = bus.read(getR16mem(src_idx));
            return;
        }
        if((opcode & 0xC7) == 0x06) {
            uint8_t dest_idx = (opcode >> 3) & 0x07;
            uint8_t imm8 = fetch8();
            setR8(dest_idx,imm8);
            return;
        }

        // 16bit arithmetic
        if((opcode & 0xCF) == 0x03) {
            uint8_t src_idx = (opcode >> 4) & 0x03;
            uint16_t operand = getR16(src_idx);
            setR16(src_idx, operand + 1);
            return;
        }
        if((opcode & 0xCF) == 0x0B) {
            uint8_t src_idx = (opcode >> 4) & 0x03;
            uint16_t operand = getR16(src_idx);
            setR16(src_idx, operand - 1);
            return;
        }
        if((opcode & 0xCF) == 0x09) {
                uint8_t src_idx = (opcode >> 4) & 0x03;
                uint16_t hl = getHL();
                uint16_t operand = getR16(src_idx);
                uint32_t result = hl + operand;
                
                setHL(result & 0xFFFF);
                
                bool h = ((hl & 0x0FFF) + (operand & 0x0FFF)) > 0x0FFF;
                bool c = result > 0xFFFF;
                updateFlags(isFlagSet(Flag::Z), false, h, c);
                return;
        }

        // 8bit arithmetic
        if((opcode & 0xC7) == 0x04) {
            uint8_t dest_idx = (opcode >> 3) & 0x07;
            uint8_t operand = getR8(dest_idx);
            uint8_t result = operand + 1;
            setR8(dest_idx,result);
            updateFlags(result == 0, false, (result & 0x0F) == 0x00, isFlagSet(Flag::C));
            return;
        }
        if((opcode & 0xC7) == 0x05) {
            uint8_t dest_idx = (opcode >> 3) & 0x07;
            uint8_t operand = getR8(dest_idx);
            uint8_t result = operand - 1;
            setR8(dest_idx,result);
            updateFlags(result == 0, true, (result & 0x0F) == 0x0F, isFlagSet(Flag::C));
            return;
        }

        //jump
        if((opcode & 0xE7) == 0x20) {
            uint8_t cond = (opcode >> 3) & 0x03;
            int8_t offset = static_cast<int8_t>(fetch8());
            if(checkCond(cond)) {
                PC += offset;
            }
            return;
        }

        switch (opcode)
        {
        //nop
        case 0x00:
        {
            return;
        }
        //ld [imm16], sp
        case 0x08:
        {
            uint16_t imm16 = fetch16();
            bus.write(imm16, SP & 0xFF);
            bus.write(imm16 + 1,  SP >> 8);
            return;
        }
        //jr imm8
        case 0x18:
        {
            int8_t offset = static_cast<int8_t>(fetch8());
            PC += offset;
            return;
        }
        //bit shift
        case 0x07: A = inst_rlc(A, false); return; 
        case 0x0F: A = inst_rrc(A, false); return; 
        case 0x17: A = inst_rl(A, false);  return; 
        case 0x1F: A = inst_rr(A, false);  return; 
        case 0x27:
        {
            uint8_t adjustment = 0;
            bool carry = isFlagSet(Flag::C);
            if(isFlagSet(Flag::N)) {
                adjustment = isFlagSet(Flag::H) ? (adjustment + 0x06) : adjustment;
                adjustment = carry ? (adjustment + 0x60) : adjustment;
                A -= adjustment;
            } else {
                adjustment = (isFlagSet(Flag::H) || (A & 0x0F) > 0x09) ? (adjustment + 0x06) : adjustment;
                if(carry || A > 0x99) {
                    carry = true;
                    adjustment += 0x60;
                }
                A += adjustment;
            }
            updateFlags(A == 0, isFlagSet(Flag::N), false, carry);
            return;
        }
        case 0x2F:
        {
            A = ~A;
            updateFlags(isFlagSet(Flag::Z),true,true,isFlagSet(Flag::C));
            return;
        }   
        case 0x37: updateFlags(isFlagSet(Flag::Z),false,false,true); return;
        case 0x3F: updateFlags(isFlagSet(Flag::Z),false,false,!isFlagSet(Flag::C)); return;
        // TODO Implement stop
        case 0x10: stop();return;
        default:
            return;
        }
    }
    // Block 1:
    if((opcode & 0xC0) == 0x40) {
        if(opcode == 0x76) {
            isHalted = true;
            return;
        }
        uint8_t dest_idx = (opcode >> 3) & 0x07;
        uint8_t src_idx = opcode & 0x07;
        setR8(dest_idx, getR8(src_idx));
        return;
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
        return;
    }

    // Block 3:
    if((opcode & 0xC0) == 0xC0) {
        // immediate ALU
        if((opcode & 0xC7) == 0xC6) {
            uint8_t operand = fetch8();
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
            return;
        } 
        // pop
        if((opcode & 0xCF) == 0xC1) { 
            
            uint8_t reg_idx = (opcode & 0x30) >> 4;

            setR16stk(reg_idx, popStack());
            return;
        } 
        // push
        if((opcode & 0xCF) == 0xC5) {
            uint8_t reg_idx = (opcode & 0x30) >> 4;
            uint16_t reg_val = getR16stk(reg_idx);
            pushStack(reg_val);
            return;
        }
        // ret cond
        if((opcode & 0xE7) == 0xC0) {
            uint8_t cond = (opcode & 0x18) >> 3;
            if(checkCond(cond)) {
                PC = popStack();
            }
            return;
        }

        // jp cond, imm16
        if((opcode & 0xE7) == 0xC2) {
            uint8_t cond = (opcode & 0x18) >> 3;
            uint16_t dest = fetch16();
            if(checkCond(cond)) {

                PC = dest;
            }
            return;
        }

        // call cond, imm16
        if((opcode & 0xE7) == 0xC4) {
            uint8_t cond = (opcode & 0x18) >> 3;
            uint16_t dest = fetch16();
            if(checkCond(cond)) {
                pushStack(PC);
                PC = dest;
            }
            return;
        }
        
        // rst tgt3
        if((opcode & 0xC7) == 0xC7) {
            uint8_t tgt3 = (opcode & 0x38);
            pushStack(PC);
            PC = tgt3;
            return;
        }

        switch (opcode)
        {
        // Control Flow
        case 0xC9: PC = popStack(); return;
        case 0xD9: PC = popStack(); IME = true; return;
        case 0xC3: PC = fetch16(); return;
        case 0xE9: PC = getHL(); return;
        case 0xCD: 
        {
            uint16_t dest = fetch16(); 
            pushStack(PC); 
            PC = dest; 
            return;
        }
        // LDH
        case 0xE2: bus.write(0xFF00 + C,A); return;
        case 0xE0: bus.write(0xFF00 + fetch8(),A); return;
        case 0xEA: bus.write(fetch16(),A); return;
        case 0xF2: A = bus.read(0xFF00 + C); return;
        case 0xF0: A = bus.read(0xFF00 + fetch8()); return;
        case 0xFA: A = bus.read(fetch16()); return;

         // SP instructions
        case 0xE8:
        {
        int8_t offset = static_cast<int8_t>(fetch8());
        bool h = ((SP & 0x0F) + (offset & 0x0F)) > 0x0F;
        bool c = ((SP & 0xFF) + (static_cast<uint8_t>(offset) & 0xFF)) > 0xFF;
        SP += offset;
        updateFlags(false, false, h, c);
        return;
        }
        case 0xF8:
        {
        int8_t offset = static_cast<int8_t>(fetch8());
        bool h = ((SP & 0x0F) + (offset & 0x0F)) > 0x0F;
        bool c = ((SP & 0xFF) + (static_cast<uint8_t>(offset) & 0xFF)) > 0xFF;
        setHL(SP + offset);
        updateFlags(false, false, h, c);
        return;
        }
        case 0xF9: SP = getHL(); return;

        // interrupts
        case 0xF3: IME = false; return;
        case 0xFB: IME = true; return;

        // Prefix
        case 0xCB:
        {
            uint8_t opcode2 = fetch8();
            if((opcode2 & 0xC0) == 0x00) {
                uint8_t src_idx = opcode2 & 0x07;
                uint8_t operand = getR8(src_idx);
                uint8_t operation = (opcode2 & 0x38) >> 3;
                switch (operation)
                {
                case 0: setR8(src_idx, inst_rlc(operand, true)); return;
                case 1: setR8(src_idx, inst_rrc(operand, true)); return;
                case 2: setR8(src_idx, inst_rl(operand, true)); return;
                case 3: setR8(src_idx, inst_rr(operand, true)); return;
                case 4:
                {
                    uint8_t MSB = operand >> 7;
                    uint8_t result = (operand << 1);
                    setR8(src_idx, result);
                    updateFlags(result == 0, false,false, MSB == 1);
                    return; 
                }
                case 5:
                {
                    uint8_t MSB = operand & 0x80;
                    uint8_t LSB = operand & 0x01;
                    uint8_t result = (operand >> 1) | MSB;
                    setR8(src_idx, result);
                    updateFlags(result == 0, false,false, LSB == 1);
                    return;  
                }
                case 6:
                {
                    uint8_t high = operand & 0xF0;
                    uint8_t low = operand & 0x0F;
                    uint8_t result = (low << 4) | (high >> 4);
                    setR8(src_idx, result);
                    updateFlags(result == 0, false,false, false);
                    return;
                }
                case 7:
                {
                    uint8_t LSB = operand & 0x01;
                    uint8_t result = (operand >> 1);
                    setR8(src_idx, result);
                    updateFlags(result == 0, false,false, LSB == 1);
                    return; 
                }
                default:
                    return;
                }
            }
            if((opcode2 & 0xC0) == 0x40) {
                uint8_t src_idx = opcode2 & 0x07;
                uint8_t operand = getR8(src_idx);
                uint8_t bit = (opcode2 >> 3) & 0x07; 
                bool isZero = !(operand & (1 << bit));
                updateFlags(isZero, false, true, isFlagSet(Flag::C)); 
                return;

            }
            if((opcode2 & 0xC0) == 0x80) {
                uint8_t src_idx = opcode2 & 0x07;
                uint8_t operand = getR8(src_idx);
                uint8_t bit = (opcode2 >> 3) & 0x07; 
                setR8(src_idx, operand & ~(1 << bit));
                return;
            }
            if((opcode2 & 0xC0) == 0xC0) {
                uint8_t src_idx = opcode2 & 0x07;
                uint8_t operand = getR8(src_idx);
                uint8_t bit = (opcode2 >> 3) & 0x07; 
                setR8(src_idx, operand | (1 << bit));
                return;
            } 
            
            return;
        }
        default:
            return;
        }

    }

}

void CPU::stop() {
    // TODO
}