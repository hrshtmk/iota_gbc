#pragma once
#include <cstdint>
#include <string>
#include <vector>
using namespace std;
class PPU;
class BUS {
    public:
        friend class PPU;
        BUS();
        bool loadRom(const std::string& filename);
        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t value);

    private:
        
        vector<uint8_t> CartridgeRom;
        uint8_t VRAMBanks[2][8192]; //Video RAM: 2 Banks of 8KB.
        uint8_t WRAMBanks[8][4096]; //Work RAM: 8 Banks of 4KB.
        uint8_t OAM[160];           //sprites.
        uint8_t IORegisters[128];   //Memory-mapped IO (0xFF00 - 0xFF7F).
        uint8_t HRAM[127];          // High RAM (0xFF80 - 0xFFFE)
        uint8_t IERegister;        // Interrupt Enable (0xFFFF)

        // Memory Bank States.
        uint8_t VRAMBankSelect;
        uint8_t WRAMBankSelect;

        // FPT for IO writes.
        typedef void (BUS::*IOWriteHandler)(uint8_t value);
        IOWriteHandler IOregisterTable[128];
        
        void InitIOHandler();
        
        void IOwrite(uint16_t address, uint8_t value);
        void DIVwrite(uint8_t value);
        void VBKwrite(uint8_t value);
        void SVBKwrite(uint8_t value);
};