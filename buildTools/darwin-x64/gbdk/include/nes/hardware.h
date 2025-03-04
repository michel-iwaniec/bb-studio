/** @file nes/hardware.h
    Defines that let the NES hardware registers be accessed
    from C.
*/
#ifndef _HARDWARE_H
#define _HARDWARE_H

#include <types.h>

#define __SHADOW_REG extern volatile uint8_t
#define __REG(addr) volatile __at (addr) uint8_t

__REG(0x2000) PPUCTRL;
#define PPUCTRL_NMI         0b10000000
#define PPUCTRL_SPR_8X8     0b00000000
#define PPUCTRL_SPR_8X16    0b00100000
#define PPUCTRL_BG_CHR      0b00010000
#define PPUCTRL_SPR_CHR     0b00001000
#define PPUCTRL_INC32       0b00000100
__SHADOW_REG shadow_PPUCTRL;

__REG(0x2001) PPUMASK;
#define PPUMASK_BLUE        0b10000000
#define PPUMASK_RED         0b01000000
#define PPUMASK_GREEN       0b00100000
#define PPUMASK_SHOW_SPR    0b00010000
#define PPUMASK_SHOW_BG     0b00001000
#define PPUMASK_SHOW_SPR_LC 0b00000100
#define PPUMASK_SHOW_BG_LC  0b00000010
#define PPUMASK_MONOCHROME  0b00000001
__SHADOW_REG shadow_PPUMASK;

__REG(0x2002) PPUSTATUS;
__REG(0x2003) OAMADDR;
__REG(0x2004) OAMDATA;
__REG(0x2005) PPUSCROLL;
__REG(0x2006) PPUADDR;
__REG(0x2007) PPUDATA;
__REG(0x4014) OAMDMA;

#define DEVICE_SCREEN_X_OFFSET 0
#define DEVICE_SCREEN_Y_OFFSET 0
#define DEVICE_SCREEN_WIDTH 32
#define DEVICE_SCREEN_HEIGHT 30
#define DEVICE_SCREEN_BUFFER_WIDTH 32
#define DEVICE_SCREEN_BUFFER_HEIGHT 30
#define DEVICE_SCREEN_MAP_ENTRY_SIZE 1
#define DEVICE_SPRITE_PX_OFFSET_X 0
#define DEVICE_SPRITE_PX_OFFSET_Y -1
#define DEVICE_WINDOW_PX_OFFSET_X 0
#define DEVICE_WINDOW_PX_OFFSET_Y 0
#define DEVICE_SCREEN_PX_WIDTH (DEVICE_SCREEN_WIDTH * 8)
#define DEVICE_SCREEN_PX_HEIGHT (DEVICE_SCREEN_HEIGHT * 8)

// Scrolling coordinates (will be written to PPUSCROLL at end-of-vblank by NMI handler)
__SHADOW_REG bkg_scroll_x;
__SHADOW_REG bkg_scroll_y;

// GBDK 8x8 mapper defines
//CFG_REG = 0x8000
//PRG_REG = 0xC000
#define CFG_CHR_A12         0b00000001
#define CFG_CHR_A13         0b00000010
#define CFG_CHR_A14         0b00000100
#define CFG_4S_DISABLE      0b00001000
#define CFG_SWP_SPR_4S      0b00010000
#define CFG_WRAM_ENABLE     0b00100000
#define CFG_WRAM_BANK       0b01000000
#define CFG_IRQ_ENABLE      0b10000000

#endif
