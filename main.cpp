#include "cpu.hpp"
#include "ppu.hpp"
#include <iostream>

int main() {
    BUS bus;
    CPU cpu;
    PPU ppu;

    bus.ConnectPPU(&ppu);

    // Load Game
    if (!bus.loadRom("cgb_boot.bin")) {
        return -1;
    }
    std::cout << "Booting Emulation Loop..." << std::endl;
    while (true) {
        uint32_t cycles_before = cpu.cycles;
        cpu.ExecNextOpcode(bus);
        uint32_t elapsed_cycles = cpu.cycles - cycles_before;

        ppu.Tick(elapsed_cycles, bus);
        // if (bus.read(0xFF44) > 0) {
        //         std::cout << "CPU read LY: " << (int)bus.read(0xFF44) << std::endl;
        // }
        if (ppu.FrameReady){
            ppu.FrameReady = false;
        }
    }
    return 0;
}