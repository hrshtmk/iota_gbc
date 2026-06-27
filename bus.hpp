#pragma once
#include <cstdint>
#include <string>
#include <vector>
using namespace std;

class BUS {
    public:
        BUS();
        bool loadRom(const std::string& filename);
        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t value);

    private:
        vector<uint8_t> Memory;
        vector<uint8_t> CartridgeRom;
};