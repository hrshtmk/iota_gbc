#include "bus.hpp"
#include "ppu.hpp"
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

void BUS::ConnectPPU(PPU* ppuPTR){
    ppu = ppuPTR;
}
// Reading value at address.
uint8_t BUS::read(uint16_t address) const {
    //ROM Space.
    if (address < 0x8000){
        if (address < CartridgeRom.size()){
            return CartridgeRom[address];
        }
        return 0xFF;
    }
    
    //VRAM Banked.
    if (address>=0x8000 && address<=0x9FFF) return VRAMBanks[VRAMBankSelect][address-0x8000];
    
    //External RAM.
    if (address>=0xA000 && address<=0xBFFF) return 0xFF;

    //WRAM Bank 0 FIXED.
    if (address>=0xC000 && address<=0xCFFF) return WRAMBanks[0][address-0xC000];
    //WRAM Bank 1-7 SWITCH.
    if (address>=0xD000 && address<=0xDFFF) return WRAMBanks[WRAMBankSelect][address-0xD000];

    //Echo RAM.
    if (address>=0xE000 && address<=0xFDFF) {
        uint16_t target = address - 0x2000;
        return read(target);
    }

    //OAM.
    if (address>=0xFE00 && address<=0xFE9F) return OAM[address-0xFE00];

    //EMPTY.
    if (address>=0xFEA0 && address<=0xFEFF) return 0xFF;

    //IO Registers.
    if (address>=0xFF00 && address<=0xFF7F){
        if (address>=0xFF40 && address <= 0xFF4B && ppu != nullptr){
            return ppu->ReadRegister(address);
        }
        return IORegisters[address&0x7F];
    }

    //HRAM.
    if (address>=0xFF80 && address<=0xFFFE) return HRAM[address-0xFF80];

    //Interrupt Enable Register.
    if (address==0xFFFF) return IERegister;

    return 0xFF;
}

// Writing value at address.
void BUS::write(uint16_t address, uint8_t value){
    //Read Only.
    if (address<0x8000) return;

    // VRAM
    if (address>=0x8000 && address<=0x9FFF) {
        VRAMBanks[VRAMBankSelect][address-0x8000] = value;
        return;
    }
    
    // External RAM
    if (address>=0xA000 && address<=0xBFFF) return;
    
    // WRAM Bank 0
    if (address>=0xC000 && address<=0xCFFF) {
        WRAMBanks[0][address-0xC000] = value;
        return;
    }
    
    // WRAM Bank 1-7
    if (address>=0xD000 && address<=0xDFFF) {
        WRAMBanks[WRAMBankSelect][address-0xD000] = value;
        return;
    }
    
    // Echo RAM
    if (address>=0xE000 && address<=0xFDFF) {
        write(address-0x2000, value);
        return;
    }
    
    // OAM
    if (address>=0xFE00 && address<=0xFE9F) {
        OAM[address-0xFE00] = value;
        return;
    }
    
    if (address>=0xFEA0 && address<=0xFEFF) return;
    
    // IO Registers
    if (address>=0xFF00 && address<=0xFF7F) {
        if (address>=0xFF40 && address<=0xFF4B && ppu != nullptr){
            ppu->WriteRegister(address, value);
            return;
        }
        uint8_t offset = address & 0x7F;
        if (IOregisterTable[offset] != nullptr) {
            (this->*IOregisterTable[offset])(value);
        } else {
            IORegisters[offset] = value;
        }
        return;
    }
    // HRAM
    if (address>=0xFF80 && address<=0xFFFE) {
        HRAM[address-0xFF80] = value;
        return;
    }
    // IE Register
    if (address==0xFFFF) {
        IERegister = value;
        return;
    }
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