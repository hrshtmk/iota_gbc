#include <cstdint>
#include <stdio.h>
#include "cpu.hpp"
#include "bus.hpp"
#include <stdlib.h>

CPU::CPU() {
    for (int i = 0; i < 256; i++) {
        OPCodeTable[i] = &CPU::UnknownOP;
    }

    #define OP(hex) OPCodeTable[0x##hex] = &CPU::op_##hex;
    #include "opcodes.def"
    #undef OP
    
    for (int i = 0; i < 256; i++) {
        CBPrefixedTable[i] = &CPU::UnknownCBOP;
    }

    #define CB(hex) CBPrefixedTable[0x##hex] = &CPU::cb_##hex;
    #include "cb_opcodes.def"
    #undef CB
}

void CPU::ExecNextOpcode(BUS &bus) {
    uint8_t opcode = bus.read(PC);
    PC++;
    // DBGprintAllOP(bus); //debug.
    (this->*OPCodeTable[opcode])(bus);
}

void CPU::UnknownOP(BUS &bus) {
    printf("\nFIX : Unknown opcode 0x%02X at 0x%04X.\n",bus.read(PC-1), PC - 1);
    
    printf("\n-----------------DEBUG REGISTERS-----------------\n");
    printf("A = 0x%02X | B = 0x%02X | C = 0x%02X\n", A, B, C);
    printf("D = 0x%02X | E = 0x%02X | F = 0x%02X\n", D, E, F);
    printf("H = 0x%02X | L = 0x%02X\n", H, L);
    printf("PC = 0x%04X | SP = 0x%04X\n", PC, SP);
    
    exit(1);
}

void CPU::UnknownCBOP(BUS &bus) {
    printf("\nFIX : Unknown CB prefixed opcode 0x%02X at 0x%04X.\n", bus.read(PC-1), PC - 1);
    exit(1);
}

uint8_t CPU::Read8bitInline(BUS &bus) {

    uint8_t byte = bus.read(PC);
    PC++; 
    
    return byte;
}

uint16_t CPU::Read16bitInline(BUS &bus) {

    uint16_t low = bus.read(PC);
    PC++;
    uint16_t high = bus.read(PC);
    PC++;
    
    return (high << 8) | low;
}
int i = 0;
void CPU::DBGprintAllOP(BUS &bus){
    printf("%d) Executing Opcode: 0x%02X at PC: 0x%04X | SP: 0x%04X | Cycles: %d\n",i, bus.read(PC-1), PC, SP, cycles);
    i++;
}
