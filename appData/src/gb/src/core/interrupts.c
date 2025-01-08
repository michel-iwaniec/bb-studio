#pragma bank 255

#include <gbdk/platform.h>

#include "interrupts.h"

#include "scroll.h"
#include "parallax.h"
#include "ui.h"
#include "palette.h"

uint16_t DIV_REG = 0; // TODO: Figure out why we get a link error for this symbol, even though gbdk-nes shouldn't need it...

//#define LYC_SYNC_VALUE 150u
#define LYC_SYNC_VALUE 0u

UBYTE hide_sprites = FALSE;
UBYTE show_actors_on_overlay = FALSE;
UBYTE overlay_cut_scanline = LYC_SYNC_VALUE;

extern uint8_t _lcd_scanline;
extern uint8_t _vram_transfer_mapper_bits;
extern UBYTE scene_SHOW_LC;
extern UBYTE scene_scroll_x_adjust;
extern int16_t scene_scroll_y_adjust;
UBYTE draw_scroll_x_adjusted;

void _add_palette_rewrite(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3);

// Adjusts scroll Y for non-power-of-two-height (basically a slow modulo)
// Also adds an extra adjustment to distribute vertical scroll seams to top / bottom
inline uint8_t adjust_scroll_y(int16_t sy)
{
    sy += scene_scroll_y_adjust;
    if(sy < 0)
    {
        while(sy < 0)
            sy += DEVICE_SCREEN_PX_HEIGHT;
    }
    else if(sy >= DEVICE_SCREEN_PX_HEIGHT)
    {
        while(sy >= DEVICE_SCREEN_PX_HEIGHT)
            sy -= DEVICE_SCREEN_PX_HEIGHT;
    }
    return sy & 0xFF;
}

void remove_LCD_ISRs(void) BANKED {
    CRITICAL {
        remove_LCD(parallax_LCD_isr);
        remove_LCD(simple_LCD_isr);
        remove_LCD(fullscreen_LCD_isr);
        //LCDC_REG &= ~LCDCF_BG8000;
    }
}

void isr_show_overlay(void) NONBANKED {
    // Write BKG palette 7 (UI) BkgPalette[7]
    _add_palette_rewrite(BkgPalette[7].c0, BkgPalette[7].c1, BkgPalette[7].c2, BkgPalette[7].c3);
    // Show window and hide sprites
    _vram_transfer_mapper_bits |= (CFG_SWP_SPR_4S | CFG_CHR_A14 | CFG_CHR_A13 | CFG_CHR_A12);
    _lcd_scanline = 0;
    move_bkg(0, 0);
    HIDE_SPRITES;
    // Show leftmost BG column unconditionally
    shadow_PPUMASK |= PPUMASK_SHOW_BG_LC;
}

void simple_LCD_isr(void) NONBANKED {
    bool is_overlay_active = win_pos_y < DEVICE_SCREEN_PX_HEIGHT;
    if(is_overlay_active) {
        if(_lcd_scanline == 0 && win_pos_y >= 3)
        {
            _lcd_scanline = win_pos_y-2;
        }
        else if(_lcd_scanline == win_pos_y-2 || win_pos_y < 3)
        {
            isr_show_overlay();
            return;
        }
    }
}

uint8_t next_is_win;

void fullscreen_LCD_isr(void) NONBANKED {
    bool is_overlay_active = win_pos_y < DEVICE_SCREEN_PX_HEIGHT-2;
    if(is_overlay_active) {
        _vram_transfer_mapper_bits &= 0xF8;
        if(next_is_win || win_pos_y < 3)
        {
            isr_show_overlay();
            return;
        }
        else
        {
            _vram_transfer_mapper_bits |= (_lcd_scanline >> 6);
            _lcd_scanline &= 0xC0;
            _lcd_scanline += 64;
            if(_lcd_scanline >= win_pos_y || _lcd_scanline == 0)
            {
                next_is_win = 1;
                // Work-around for bug with overlay palette-rewrite +1 scanline after regular scroll split
                uint8_t _lcd_scanline_previous = _lcd_scanline - 64;
                if(_lcd_scanline_previous == (UBYTE)(win_pos_y-1))
                {
                    // Push palette rewrite one scanline down to occur 2 scanlines down instead
                    _lcd_scanline = win_pos_y + 1;
                }
                else
                {
                    _lcd_scanline = win_pos_y;
                }
            }
        }
    }
    else
    {
        _vram_transfer_mapper_bits &= 0xF8;
        _vram_transfer_mapper_bits |= (_lcd_scanline >> 6);
        _lcd_scanline += 64;
    }
}

void VBL_isr(void) NONBANKED {
    draw_scroll_x_adjusted = (uint8_t)draw_scroll_x + scene_scroll_x_adjust;
    if (hide_sprites)
    {
        HIDE_SPRITES;
    }
    else
    {
        SHOW_SPRITES;
    }
    scroll_shadow_update();
    _lcd_scanline = 0;
    move_bkg(draw_scroll_x_adjusted, adjust_scroll_y(draw_scroll_y));
    music_play_isr();
    // Hide window
    _vram_transfer_mapper_bits &= ~(CFG_SWP_SPR_4S | CFG_CHR_A14 | CFG_CHR_A13 | CFG_CHR_A12);
    // Hide leftmost BG/SPR column depending on scene setting
    shadow_PPUMASK &= ~(PPUMASK_SHOW_BG_LC | PPUMASK_SHOW_SPR_LC);
    shadow_PPUMASK |= scene_SHOW_LC;
    next_is_win = 0;
}
