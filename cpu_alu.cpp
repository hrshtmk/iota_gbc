#include "cpu.hpp"
#include <cstdint>

void CPU::op_00(BUS &bus){ //NOP
    cycles += 4;
}
void CPU::op_31(BUS &bus){ //LD SP, u16
    SP = Read16bitInline(bus);
    cycles += 12;
}
void CPU::op_3E(BUS &bus){ //LD A, u8
    A = Read8bitInline(bus);
    cycles += 8;
}
void CPU::op_C3(BUS &bus){ //JP u16
    PC = Read16bitInline(bus);
    cycles += 16;
}
void CPU::op_E0(BUS &bus){ //LD (FF00+u8), A
    uint8_t offset = Read8bitInline(bus);
    uint16_t target = 0xFF00 + offset;
    bus.write(target, A);
    cycles += 12;
}
void CPU::op_CD(BUS &bus){ //CALL u16
    uint16_t target = Read16bitInline(bus);
    uint16_t ret_add = PC;

    SP--;
    bus.write(SP, (ret_add >> 8) & 0xFF);
    SP--;
    bus.write(SP, ret_add & 0xFF);
    PC = target;
    cycles += 24;
}
void CPU::op_A5(BUS &bus){ //AND A, L
    A = A & L;
    uint8_t zero_flag = (A==0) ? 0x80 : 0x00;
    uint16_t half_carry = 0x20;
    F = zero_flag | half_carry;

    cycles += 4;
}
void CPU::op_B9(BUS &bus){ //CP A, C
    uint8_t zero_flag = (A == C) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((A & 0x0F) < (C & 0x0F)) ? 0x20 : 0x00;
    uint8_t carry_flag = (A < C) ? 0x10 : 0x00;
    F = zero_flag | subtract_flag | half_carry | carry_flag;

    cycles += 4; 
}
void CPU::op_42(BUS &bus){ // LD B, D
    B = D;
    cycles += 4;
}
void CPU::op_3C(BUS &bus){ //INC A
    uint8_t prev_A = A;
    A++;
    
    uint8_t zero_flag = (A==0) ? 0x80 : 0x00;
    uint8_t half_carry = ((prev_A & 0x0F) == 0x0F) ? 0x20 : 0x00;
    F = (F & 0x10) | zero_flag | half_carry;
    
    cycles += 4;
}
void CPU::op_58(BUS &bus){ //LD E, B
    E = B;
    cycles += 4;
}
void CPU::op_43(BUS &bus){ //LD B, E
    B = E;
    cycles += 4;
}
void CPU::op_21(BUS &bus){ //LD HL, u16
    HL = Read16bitInline(bus);
    cycles += 12;
}
void CPU::op_AF(BUS &bus){ //XOR A, A
    A = A ^ A;
    F = 0x80;
    cycles += 4;
}
void CPU::op_0E(BUS &bus){ //LD C, u8
    C = Read8bitInline(bus);
    cycles += 8;
}
void CPU::op_22(BUS &bus){ //LD (HL+), A
    bus.write(HL, A);
    HL++;
    cycles += 8;
}
void CPU::op_C9(BUS &bus){ //RET
    uint16_t low = bus.read(SP);
    SP++;
    uint16_t high = bus.read(SP);
    SP++;
    PC = (high << 8) | low;
    
    cycles += 16;
}
void CPU::op_2F(BUS &bus){ //CPL
    A = ~A;
    F |= 0x40;
    F |= 0x20;
    
    cycles += 4;
}
void CPU::op_0D(BUS &bus){ // DEC C
    uint8_t prev_C = C;
    C--;
    
    uint8_t zero_flag = (C == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((prev_C & 0x0F) == 0x00) ? 0x20 : 0x00; 
    
    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;
    
    cycles += 4;
}
void CPU::op_20(BUS &bus){ //JR NZ, i8
    int8_t offset = static_cast<int8_t>(Read8bitInline(bus));
    bool condition_met = ((F & 0x80)==0);
    if (condition_met) {
        PC += offset;
        cycles += 12;
    } 
    else {
        cycles += 8;
    }
}
