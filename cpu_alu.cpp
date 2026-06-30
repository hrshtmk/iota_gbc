#include "bus.hpp"
#include "cpu.hpp"
#include <cstdint>
#include <sched.h>
#include <sys/types.h>

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
void CPU::op_CB(BUS &bus) { //CB Prefix
    uint8_t cb_opcode = Read8bitInline(bus);
    (this->*CBPrefixedTable[cb_opcode])(bus);
}
void CPU::cb_6C(BUS &bus) { // BIT 5, H
    uint8_t zero_flag = ((H & (1 << 5)) == 0) ? 0x80 : 0x00;
    F = (F & 0x10) | zero_flag | 0x20;

    cycles += 8;
}
void CPU::op_28(BUS &bus){ //JR Z, i8
    int8_t offset = static_cast<int8_t>(Read8bitInline(bus));
    bool condition_met = ((F & 0x80) != 0);

    if (condition_met) {
        PC += offset;
        cycles += 12;
    }
    else {
        cycles += 8;
    }
}
void CPU::op_26(BUS &bus){ //LD H, u8
    H = Read8bitInline(bus);
    cycles += 8;
}
void CPU::op_11(BUS &bus){ //LD DE, u16
    DE = Read16bitInline(bus);
    cycles += 12;
}
void CPU::op_4C(BUS &bus){ //LD C, H
    C = H;
    cycles += 4;
}
void CPU::op_1A(BUS &bus) { //LD A, (DE)
    A = bus.read(DE);
    cycles += 8;
}
void CPU::op_E2(BUS &bus){ //LD (FF00+C), A
    bus.write(0xFF00+C, A);
    cycles += 8;
}
void CPU::op_0C(BUS &bus){ //INC C
    uint8_t prev_C = C;
    C++;
    
    uint8_t zero_flag = (C==0) ? 0x80 : 0x00;
    uint8_t half_carry = ((prev_C & 0x0F) == 0x0F) ? 0x20 : 0x00;
    F = (F & 0x10) | zero_flag | half_carry;
    
    cycles += 4;
}
void CPU::op_47(BUS &bus) { //LD B, A
    B = A;
    cycles += 4;
}
void CPU::op_D5(BUS &bus){ // PUSH DE
    bus.write(--SP, D);
    bus.write(--SP, E);
    cycles += 16;
}
void CPU::op_16(BUS &bus) { //LD D, u8
    D = Read8bitInline(bus);
    cycles += 8;
}
void CPU::cb_10(BUS &bus) { // RL B
    uint8_t oldCarry = (F & 0x10) ? 1 : 0;
    uint8_t newCarry = (B & 0x80) ? 0x10 : 0x00;

    B = (B << 1) | oldCarry;

    F = (B == 0 ? 0x80 : 0x00) | newCarry;

    cycles += 8;
}
void CPU::op_17(BUS &bus){ // RLA
    uint8_t oldCarry = (F & 0x10) ? 1 : 0;
    uint8_t newCarry = (A & 0x80) ? 0x10 : 0x00;

    A = (A << 1) | oldCarry;
    F = newCarry;

    cycles += 4;
}
void CPU::cb_13(BUS &bus){ // RL E
    uint8_t oldCarry = (F & 0x10) ? 1 : 0;
    uint8_t newCarry = (E & 0x80) ? 0x10 : 0x00;

    E = (E << 1) | oldCarry;
    uint8_t zero_flag = (E == 0) ? 0x80 : 0x00;
    F = zero_flag | newCarry;

    cycles += 8;
}
void CPU::op_15(BUS &bus){ // DEC D
    uint8_t prev_D = D;
    D--;
    
    uint8_t zero_flag = (D == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((prev_D & 0x0F) == 0x00) ? 0x20 : 0x00; 
    
    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;
    
    cycles += 4;
}
void CPU::op_D1(BUS &bus){ // POP DE
    E = bus.read(SP++);
    D = bus.read(SP++);
    cycles += 12;
}
void CPU::op_23(BUS &bus){ //INC HL
    HL++;
    cycles += 8;
}
void CPU::op_13(BUS &bus){ //INC DE
    DE++;
    cycles += 8;
}
void CPU::op_7B(BUS &bus){ //LD A, E
    A = E;
    cycles += 4;
}
void CPU::op_FE(BUS &bus){ //CP A, u8
    uint8_t u8 = Read8bitInline(bus);
    uint8_t zero_flag = (A == u8) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((A & 0x0F) < (u8 & 0x0F)) ? 0x20 : 0x00;
    uint8_t carry_flag = (A < u8) ? 0x10 : 0x00;
    F = zero_flag | subtract_flag | half_carry | carry_flag;

    cycles += 8;
}
void CPU::op_44(BUS &bus){ //LD B, H
    B = H;
    cycles += 4;
}
void CPU::op_34(BUS &bus){ // INC (HL)
    uint8_t val = bus.read(HL);
    uint8_t res = val + 1;
    bus.write(HL, res);
    F = (F & 0x10) | (res == 0 ? 0x80 : 0x00) | ((val & 0x0F) == 0x0F ? 0x20 : 0x00);
    cycles += 12;
}
void CPU::op_06(BUS &bus){ //LD B, u8
    B = Read8bitInline(bus);
    cycles += 8;
}
void CPU::op_05(BUS &bus){ //DEC B
    uint8_t prev_B = B;
    B--;
    
    uint8_t zero_flag = (B == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((prev_B & 0x0F) == 0x00) ? 0x20 : 0x00; 
    
    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;
    
    cycles += 4;
}
void CPU::op_E6(BUS &bus){ //AND A, u8
    uint8_t u8 = Read8bitInline(bus);
    A = A & u8;
    uint8_t zero_flag = (A==0) ? 0x80 : 0x00;
    uint16_t half_carry = 0x20;
    F = zero_flag | half_carry;

    cycles += 8;
}
void CPU::op_F0(BUS &bus){ //LD A,(FF00+u8)
    uint8_t offset = Read8bitInline(bus);
    A = bus.read(0xFF00+offset);
    cycles += 12;
}
void CPU::op_49(BUS &bus){ //LD C, C (NOP-like)
    cycles += 4;
}
void CPU::op_7E(BUS &bus){ //LD A,(HL)
    A = bus.read(HL);
    cycles += 8;
}
void CPU::op_B0(BUS &bus){ //OR A, B
    A = A | B;
    uint8_t zero_flag = (A==0) ? 0x80 : 0x00;
    F = zero_flag;
    cycles += 4;
}
void CPU::op_0F(BUS &bus){ //RRCA
    uint8_t carry_flag = A & 0x01;
    A = (A >> 1) | (carry_flag << 7);
    F = (carry_flag) ? 0x10 : 0x00;
    cycles += 4;
}
void CPU::cb_49(BUS &bus){ //BIT 1, C
    uint8_t zero_flag = ((C & 0x02) == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x20; 

    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;

    cycles += 8;
}
void CPU::cb_37(BUS &bus){ //SWAP A
    uint8_t prev_A = A;
    A = (prev_A >> 4) | (prev_A << 4);

    uint8_t zero_flag = (A == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x00;

    F = zero_flag | subtract_flag | half_carry;

    cycles += 8;
}
void CPU::cb_41(BUS &bus){ //BIT 0, C
    uint8_t zero_flag = ((C & 0x01) == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x20; 

    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;

    cycles += 8;
}
void CPU::op_79(BUS &bus){
    A = C;
    cycles += 4;
}
void CPU::op_18(BUS &bus){ //JR i8
    int8_t offset = static_cast<int8_t>(Read8bitInline(bus));
    PC += offset;
    cycles += 12;
}
void CPU::op_3D(BUS &bus){ //DEC A
    uint8_t prev_A = A;
    A--;
    
    uint8_t zero_flag = (A == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((prev_A & 0x0F) == 0x00) ? 0x20 : 0x00; 
    
    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;
    
    cycles += 4;
}
void CPU::op_01(BUS &bus){ //LD BC, u16
    BC = Read16bitInline(bus);
    cycles += 12;
}
void CPU::op_09(BUS &bus) { // ADD HL, BC
    uint32_t val1 = HL;
    uint32_t val2 = BC;
    uint32_t result = val1 + val2;

    bool half_carry = ((val1 & 0x0FFF) + (val2 & 0x0FFF)) > 0x0FFF;
    bool carry = result > 0xFFFF;

    HL = static_cast<uint16_t>(result);
    
    F = (F & 0x80) | (half_carry ? 0x20 : 0x00) | (carry ? 0x10 : 0x00);

    cycles += 8;
}
void CPU::op_19(BUS &bus) { // ADD HL, DE
    uint32_t val1 = HL;
    uint32_t val2 = DE;
    uint32_t result = val1 + val2;

    bool half_carry = ((val1 & 0x0FFF) + (val2 & 0x0FFF)) > 0x0FFF;
    bool carry = result > 0xFFFF;

    HL = static_cast<uint16_t>(result);

    F = (F & 0x80) | 
        (half_carry ? 0x20 : 0x00) | 
        (carry      ? 0x10 : 0x00);

    cycles += 8;
}
void CPU::op_02(BUS &bus){ //LD (BC), A
    bus.write(BC, A);
    cycles += 8;
}
void CPU::op_2E(BUS &bus){ // LD L, u8
    L = Read8bitInline(bus);
    cycles += 8;
}
void CPU::op_12(BUS &bus){ //LD (DE), A
    bus.write(DE, A);
    cycles += 8;
}
void CPU::op_2A(BUS &bus){ //LD A,(HL+)
    A = bus.read(HL);
    HL++;
    cycles += 8;
}
void CPU::op_C5(BUS &bus){ // PUSH BC
    bus.write(--SP, B);
    bus.write(--SP, C);
    cycles += 16;
}
void CPU::op_E5(BUS &bus){ // PUSH HL
    bus.write(--SP, H);
    bus.write(--SP, L);
    cycles += 16;   
}
void CPU::op_1E(BUS &bus){ // LD E, u8
    E = Read8bitInline(bus);
    cycles += 8;
}
void CPU::op_4A(BUS &bus){ //LD C, D
    C = D;
    cycles += 4;
}
void CPU::op_E1(BUS &bus){ //POP HL
    L = bus.read(SP++);
    H = bus.read(SP++);
    cycles += 12; 
}
void CPU::op_C1(BUS &bus){ //POP BC
    C = bus.read(SP++);
    B = bus.read(SP++);
    cycles += 12; 
}
void CPU::op_BB(BUS &bus){ //CP A, E
    uint8_t zero_flag = (A == E) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((A & 0x0F) < (E & 0x0F)) ? 0x20 : 0x00;
    uint8_t carry_flag = (A < E) ? 0x10 : 0x00;
    F = zero_flag | subtract_flag | half_carry | carry_flag;

    cycles += 4;
}
void CPU::op_EA(BUS &bus){ // LD (u16), A
    uint16_t address = Read16bitInline(bus);
    bus.write(address, A);
    
    cycles += 16;
}
void CPU::op_07(BUS &bus){ // RLCA
    uint8_t carry_flag = (A & 0x80) ? 0x10 : 0x00;

    A = (A << 1) | ((A & 0x80) >> 7);
    F = carry_flag;

    cycles += 4;
}
void CPU::op_FA(BUS &bus){ //LD A, (u16)
    uint16_t target = Read16bitInline(bus);
    A = bus.read(target);

    cycles += 16;
}
void CPU::cb_40(BUS &bus){ //BIT 0, B
    uint8_t zero_flag = ((B & 0x01) == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x20; 

    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;

    cycles += 8;
}
void CPU::cb_48(BUS &bus){ //BIT 1, B
    uint8_t zero_flag = ((B & 0x02) == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x20; 

    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;

    cycles += 8;
}
void CPU::cb_50(BUS &bus){ //BIT 2, B
    uint8_t zero_flag = ((B & 0x04) == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x20; 

    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;

    cycles += 8;
}
void CPU::op_4F(BUS &bus){
    C = A;
    cycles += 4;
}
void CPU::cb_86(BUS &bus){ // RES 0, (HL)
    uint8_t value = bus.read(HL);
    value &= ~0x01;
    bus.write(HL, value);

    cycles += 16;
}
void CPU::cb_46(BUS &bus){ //BIT 0, (HL)
    uint8_t target = bus.read(HL);
    uint8_t zero_flag = ((target & 0x01) == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x20; 

    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;

    cycles += 12;
}
void CPU::cb_7E(BUS&bus){ //BIT 7, (HL)
    uint8_t value = bus.read(HL);
    uint8_t bit7 = value & 0x80;

    uint8_t zero_flag = (bit7 == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x00;
    uint8_t half_carry = 0x20;
    F = (F & 0x10) | zero_flag | subtract_flag | half_carry;

    cycles += 12;    
}
void CPU::op_CC(BUS &bus){ //CALL Z, u16
    uint16_t target = Read16bitInline(bus);
    if ((F&0x80)!=0){
        uint16_t ret_add = PC;
        SP--;
        bus.write(SP, (ret_add>>8)&0xFF);
        SP--;
        bus.write(SP, ret_add & 0xFF);
        PC = target;
        cycles += 24;
    }
    else cycles += 12;
}
void CPU::op_A1(BUS &bus){ //AND A, C
    A = A & C;
    uint8_t zero_flag = (A==0) ? 0x80 : 0x00;
    uint16_t half_carry = 0x20;
    F = zero_flag | half_carry;

    cycles += 4;
}
void CPU::op_A9(BUS &bus){ //XOR A, C
    A = A ^ C;
    uint8_t zero_flag = (A == 0) ? 0x80 : 0x00;
    F = zero_flag;
    cycles += 4;
}
void CPU::op_32(BUS &bus){ //LD (HL-), A
    bus.write(HL, A);
    HL--;
    cycles += 8;
}
void CPU::op_77(BUS &bus){ //LD (HL), A
    bus.write(HL, A);
    cycles += 8;
}
void CPU::op_78(BUS &bus){ //LD A, B
    A = B;
    cycles += 4;
}
void CPU::op_04(BUS &bus){ //INC B
    uint8_t prev_B = B;
    B++;
    
    uint8_t zero_flag = (B==0) ? 0x80 : 0x00;
    uint8_t half_carry = ((prev_B & 0x0F) == 0x0F) ? 0x20 : 0x00;
    F = (F & 0x10) | zero_flag | half_carry;
    
    cycles += 4;
}
void CPU::op_D6(BUS &bus) { // SUB A, u8
    uint8_t value = Read8bitInline(bus);
    uint8_t result = A - value;

    uint8_t zero_flag = (result == 0) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((A & 0x0F) < (value & 0x0F)) ? 0x20 : 0x00;
    uint8_t carry_flag = (A < value) ? 0x10 : 0x00;

    A = result;
    F = zero_flag | subtract_flag | half_carry | carry_flag;

    cycles += 8;
}
void CPU::op_D7(BUS &bus) { // RST 10h
    uint16_t ret_add = PC;
    SP--;
    bus.write(SP, (ret_add >> 8) & 0xFF);
    SP--;
    bus.write(SP, ret_add & 0xFF);
    
    PC = 0x0010; // NOTE: for other RST NNh, jut change the PC = 0x00NN.

    cycles += 16;
}
void CPU::op_D2(BUS &bus) { // JP NC, u16
    uint16_t target = Read16bitInline(bus);
    if ((F & 0x10) == 0) {
        PC = target;
        cycles += 16;
    } else {
        cycles += 12;
    }
}
void CPU::op_C2(BUS &bus) { // JP NZ, u16
    uint16_t target = Read16bitInline(bus);
    if ((F & 0x80) == 0) {
        PC = target;
        cycles += 16;
    } else {
        cycles += 12;
    }
}
void CPU::op_CA(BUS &bus){ //JP Z, u16
    uint16_t target = Read16bitInline(bus);
    if ((F & 0x80) != 0) {
        PC = target;
        cycles += 16;
    } else {
        cycles += 12;
    }
}
void CPU::op_7D(BUS &bus){ //LD A, L
    A = L;
    cycles += 4;
}
void CPU::op_B1(BUS &bus){ //OR A, C
    A = A | C;
    uint8_t zero_flag = (A==0) ? 0x80 : 0x00;
    F = zero_flag;
    cycles += 4;
}
void CPU::op_F2(BUS &bus) { // LD A, (FF00+C)
    uint16_t target = 0xFF00 + C;
    A = bus.read(target);

    cycles += 8;
}
void CPU::op_BE(BUS &bus){ //CP A, (HL)
    uint8_t target = bus.read(HL);
    uint8_t zero_flag = (A == target) ? 0x80 : 0x00;
    uint8_t subtract_flag = 0x40;
    uint8_t half_carry = ((A & 0x0F) < (target & 0x0F)) ? 0x20 : 0x00;
    uint8_t carry_flag = (A < target) ? 0x10 : 0x00;
    F = zero_flag | subtract_flag | half_carry | carry_flag;

    cycles += 4;
}