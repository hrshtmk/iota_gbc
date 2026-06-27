#include "bus.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

// Initializing flat 64KB memory with 0s.
BUS::BUS(){ Memory.resize(65536, 0x00);}

// Reading value at address.
uint8_t BUS::read(uint16_t address) const {return Memory[address];}

// Writing value at address.
void BUS::write(uint16_t address, uint8_t value){
    if (address < 0x8000) return;
    Memory[address] = value;
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
    
    size_t bytesToCopy = min(static_cast<size_t>(size), static_cast<size_t>(32768));
    for (size_t i = 0; i < bytesToCopy; ++i) {
        Memory[i] = CartridgeRom[i];
    }

    return true;
}