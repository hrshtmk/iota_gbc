#include "bus.hpp"
#include <cstdint>
#include <vector>

class BUS;

enum class PPUMode: uint8_t {
    HBlank = 0,
    VBlank = 1,
    OAMSearch = 2,
    PixelTransfer = 3  
};

class PPU {
    public:
        PPU();
        ~PPU() = default;

        void Tick(uint32_t cycles, BUS &bus);
        uint8_t ReadRegister(uint16_t address) const;
        void WriteRegister(uint16_t address, uint8_t value);

        const uint32_t* GetFrameBuffer() const {return FrameBuffer;}
        bool FrameReady = false;

    private:
        static constexpr int WIDTH = 160;
        static constexpr int HEIGHT = 144;

        uint32_t FrameBuffer[WIDTH*HEIGHT];

        uint32_t DotClock = 0;


        //LCD Registers.
        uint8_t LCDC = 0x91; //control
        uint8_t STAT = 0x85; //status
        uint8_t SCY = 0x00; // Y scroll
        uint8_t SCX = 0x00; // X scroll
        uint8_t LY = 0x00; // Curr. scanline
        uint8_t LYC = 0x00; // LY compare
        uint8_t BGP = 0xFC; // Background data (NON-CGB) 
        uint8_t OBP0 = 0xFF; //OBJ Palette 0
        uint8_t OBP1 = 0xFF; //OBJ Palette 1
        uint8_t WY = 0x00; //Window Y
        uint8_t WX = 0x00; //Window X

        PPUMode GetMode() const {return static_cast<PPUMode>(STAT&0x03);}
        void SetMode(PPUMode mode);

        void RenderScanline(BUS& bus);
        void RequestInterrupt(BUS &bus, uint8_t InterruptBit);
};