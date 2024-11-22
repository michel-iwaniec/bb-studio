#pragma bank 255

#include <gbdk/platform.h>

#include "interrupts.h"

#include "scroll.h"
#include "parallax.h"
#include "ui.h"

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

void simple_LCD_isr(void) NONBANKED {
    bool is_overlay_active = win_pos_y < 144;
    if(is_overlay_active) {
        uint8_t win_end = 144 - win_pos_y;
        if(_lcd_scanline == 0)
        {
            if(win_end <= 1 || win_end >= 240)
                _lcd_scanline = 240; // skip overlay if less than 2 scanlines high
            else
                _lcd_scanline = OVERLAY_START_Y;
            return;
        }
        else if(_lcd_scanline == OVERLAY_START_Y)
        {
            // Show window and hide sprites
            _vram_transfer_mapper_bits |= (CFG_SWP_SPR_4S | CFG_CHR_A14 | CFG_CHR_A13 | CFG_CHR_A12);
            _lcd_scanline = win_end + OVERLAY_START_Y;
            move_bkg(0, 0);
            HIDE_SPRITES;
            // Show leftmost BG column unconditionally
            shadow_PPUMASK |= PPUMASK_SHOW_BG_LC;
        }
        else
        {
            INT16 draw_scroll_win_end_y = draw_scroll_y + win_end + OVERLAY_START_Y;
            // Hide window and show sprites
            _vram_transfer_mapper_bits &= ~(CFG_SWP_SPR_4S | CFG_CHR_A14 | CFG_CHR_A13 | CFG_CHR_A12);
            _lcd_scanline = OVERLAY_START_Y;
            move_bkg(draw_scroll_x_adjusted, adjust_scroll_y(draw_scroll_win_end_y));
            SHOW_SPRITES;
            // Hide leftmost BG/SPR column depending on scene setting
            shadow_PPUMASK &= ~(PPUMASK_SHOW_BG_LC | PPUMASK_SHOW_SPR_LC);
            shadow_PPUMASK |= scene_SHOW_LC;
        }
    }
    else
    {
        move_bkg(draw_scroll_x_adjusted, adjust_scroll_y(draw_scroll_y + _lcd_scanline));
        // Hide leftmost BG/SPR column depending on scene setting
        shadow_PPUMASK &= ~(PPUMASK_SHOW_BG_LC | PPUMASK_SHOW_SPR_LC);
        shadow_PPUMASK |= scene_SHOW_LC;
    }
}

void fullscreen_LCD_isr(void) NONBANKED {
    if (_lcd_scanline == LYC_SYNC_VALUE) {
        move_bkg(draw_scroll_x_adjusted, draw_scroll_y);
        _lcd_scanline = (9 * 8) - 1;
    } else {
        _lcd_scanline = LYC_SYNC_VALUE;
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
    // Hide leftmost BG/SPR column depending on scene setting
    shadow_PPUMASK &= ~(PPUMASK_SHOW_BG_LC | PPUMASK_SHOW_SPR_LC);
    shadow_PPUMASK |= scene_SHOW_LC;
}
