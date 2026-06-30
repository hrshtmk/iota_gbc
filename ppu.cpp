#include "ppu.hpp"
#include "bus.hpp"
#include <cstdint>
#include <algorithm>
#include <iostream>

PPU::PPU(){
    //Placeholder color.
    fill(begin(FrameBuffer), end(FrameBuffer), 0x555555FF);
}
void PPU::SetMode(PPUMode mode){
    STAT = (STAT & 0xF4) | static_cast<uint8_t>(mode);
}
void PPU::RequestInterrupt(BUS&bus, uint8_t InterruptBit){
    uint8_t if_reg = bus.read(0xFF0F);
    bus.write(0xFF0F, if_reg | (1 << InterruptBit));
}

void PPU::Tick(uint32_t cycles, BUS&bus){
    DotClock += cycles;

    if (LY<144){
        if (DotClock<80){
            if(GetMode() != PPUMode::OAMSearch){
                SetMode(PPUMode::OAMSearch);
                if (STAT & 0x20) RequestInterrupt(bus,1);
            }
        }
        else if (DotClock < 252){
            if (GetMode()!=PPUMode::PixelTransfer) SetMode(PPUMode::PixelTransfer);
        }
        else if (DotClock < 456){
            if (GetMode()!= PPUMode::HBlank){
                SetMode(PPUMode::HBlank);
                if (STAT&0x08) RequestInterrupt(bus, 1);

                if (DotClock>=252 && DotClock-cycles < 252){
                    RenderScanline(bus);
                }
            }
        }
    }
    else {
        if (GetMode()!=PPUMode::VBlank){
            SetMode(PPUMode::VBlank);
            RequestInterrupt(bus, 0);
            if (STAT & 0x10) RequestInterrupt(bus, 1);
            FrameReady = 1;
        }
    }
    if (DotClock >= 456){
        DotClock -= 456;
        LY++;

        if (LY == LYC){
            STAT |= 0x04;
            if (STAT & 0x40) RequestInterrupt(bus, 1);
        }
        else {
            STAT &= ~0x04;
        }
    }

    if (LY>153){
        LY = 0;
        FrameReady = false;
    }
}

void PPU::RenderScanline(BUS& bus) {
    // Placeholder, debug only for now.
    for (int x = 0; x < WIDTH; ++x) {
        uint8_t r = x * 1.5;
        uint8_t g = LY * 1.5;
        uint8_t b = 128;
        
        FrameBuffer[LY*WIDTH+x] = (r<<24) | (g<<16) | (b<<8) | 0xFF;
    }
}

// Memory-Mapped IO routing helpers
uint8_t PPU::ReadRegister(uint16_t address) const {
    switch (address) {
        case 0xFF40: return LCDC;
        case 0xFF41: return STAT;
        case 0xFF42: return SCY;
        case 0xFF43: return SCX;
        case 0xFF44: return LY;
        case 0xFF45: return LYC;
        case 0xFF47: return BGP;
        case 0xFF48: return OBP0;
        case 0xFF49: return OBP1;
        case 0xFF4A: return WY;
        case 0xFF4B: return WX;
        default: return 0xFF;
    }
}

void PPU::WriteRegister(uint16_t address, uint8_t value) {
    switch (address) {
        case 0xFF40: LCDC = value; break;
        case 0xFF41: STAT = (STAT & 0x07) | (value & 0xF8); break; // Lower 3 bits are read-only
        case 0xFF42: SCY = value; break;
        case 0xFF43: SCX = value; break;
        case 0xFF44: break; // LY is read-only from CPU perspective
        case 0xFF45: LY = value; break;
        case 0xFF47: BGP = value; break;
        case 0xFF48: OBP0 = value; break;
        case 0xFF49: OBP1 = value; break;
        case 0xFF4A: WY = value; break;
        case 0xFF4B: WX = value; break;
    }
}