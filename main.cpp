#include "cpu.hpp"

int main() {
    BUS bus;
    CPU cpu;

    // Load Game
    if (!bus.loadRom("cgb_boot.bin")) {
        return -1;
    }
    
    while (true) {
        cpu.ExecNextOpcode(bus);
    }

    return 0;
}