#include "bus.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

BUS::BUS(){
    VRAMBankSelect = 0;
    WRAMBankSelect = 1;
    IERegister = 0x00;

    //Zeroing buffers.
    fill(&VRAMBanks[0][0], &VRAMBanks[0][0]+(2*8192), 0x00);
    fill(&WRAMBanks[0][0], &WRAMBanks[0][0]+(8*4096), 0x00);
    fill(OAM, OAM+160, 0x00);
    fill(IORegisters, IORegisters+128, 0x00);
    fill(HRAM, HRAM+127, 0x00);
    
    InitIOHandler();
}

void BUS::InitIOHandler(){
    for (int i = 0; i < 128; i++) {
        IOregisterTable[i] = nullptr;
    }
    IOregisterTable[0x04] = &BUS::DIVwrite;
    IOregisterTable[0x4F] = &BUS::VBKwrite;
    IOregisterTable[0x70] = &BUS::SVBKwrite;

}

void BUS::DIVwrite(uint8_t value) {
    IORegisters[0x04] = 0x00; // Any write to DIV resets it to 0.
}

void BUS::VBKwrite(uint8_t value) {
    VRAMBankSelect = value & 0x01; // GBC VRAM bank 0 or 1.
    IORegisters[0x4F] = value;
}

void BUS::SVBKwrite(uint8_t value) {
    uint8_t bank = value & 0x07;
    if (bank == 0) bank = 1; //bank 0 acts as bank 1.
    WRAMBankSelect = bank;
    IORegisters[0x70] = value;
}

// Reading value at address.
uint8_t BUS::read(uint16_t address) const {
    //TODO with Banking.
}

// Writing value at address.
void BUS::write(uint16_t address, uint8_t value){
    //TODO.
}

// File handling.
bool BUS::loadRom(const string& filename) {
    ifstream file(filename, ios::binary | ios::ate);
    
    if (!file.is_open()) {
        cerr << "Failed to open ROM: " << filename << endl;
        return false;
    }

    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    CartridgeRom.resize(size);
    if (!file.read(reinterpret_cast<char*>(CartridgeRom.data()), size)) {
        cerr << "Failed to read ROM data." << endl;
        return false;
    }

    cout << "\n[Loaded " << filename << " of " << size << " bytes]\n";
    return true;
}