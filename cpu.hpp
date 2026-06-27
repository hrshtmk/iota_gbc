#pragma once
#include "bus.hpp"
#include <cstdint>
#define op(hex) void op_#hex
class BUS;

class CPU{
    public:
        uint32_t cycles = 0;
        void ExecNextOpcode(BUS &bus);
        CPU();
        
    private:
        using OPDef = void (CPU::*)(BUS&);
        OPDef OPCodeTable[256];
        void DBGprintAllOP(BUS &bus);
        uint16_t Read16bitInline(BUS &bus);
        uint8_t Read8bitInline(BUS &bus);
        void UnknownOP(BUS &bus);

        #define OP(hex) void op_##hex(BUS &bus);
        #include "opcodes.def"
        #undef OP
        
        // Registers.
        union {
            struct {uint8_t C; uint8_t B;};
            uint16_t BC;
        };
        union {
            struct {uint8_t E; uint8_t D;};
            uint16_t DE;
        };
        union {
            struct {uint8_t L; uint8_t H;};
            uint16_t HL;
        };
        union {
            struct {uint8_t F; uint8_t A;};
            uint16_t AF;
        };
        uint16_t PC = 0x0000; // program counter.
        uint16_t SP = 0x0000; // stack pointer.
};