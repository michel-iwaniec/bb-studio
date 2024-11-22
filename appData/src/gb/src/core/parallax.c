#pragma bank 255

#include <gbdk/platform.h>

#include <string.h>

#include "scroll.h"
#include "parallax.h"
#include "interrupts.h"
#include "ui.h"

parallax_row_t parallax_rows[3];
parallax_row_t * parallax_row;

static const parallax_row_t parallax_rows_defaults[3] = { PARALLAX_STEP(0, 2, 2), PARALLAX_STEP(2, 4, 1), PARALLAX_STEP(4, 0, 0)}; 

void parallax_init(void) BANKED {
    memcpy(parallax_rows, parallax_rows_defaults, sizeof(parallax_rows)); 
}

extern uint8_t _lcd_scanline;

void parallax_LCD_isr(void) NONBANKED {
    uint8_t y;
    bool is_overlay_active = win_pos_y < 144;
    if(is_overlay_active) {
        uint8_t win_height = 144 - win_pos_y;
        if(_lcd_scanline == 0)
        {
            move_bkg(parallax_row->scx, draw_scroll_y);
            _lcd_scanline = OVERLAY_START_Y;
            return;
        }
        else if(_lcd_scanline == OVERLAY_START_Y)
        {
            // Run the simple_LCD_isr for handling the overlay logic
            simple_LCD_isr();
            return;
        }
        else if(_lcd_scanline == OVERLAY_START_Y + win_height)
        {
            // Run the simple_LCD_isr for handling the overlay logic
            simple_LCD_isr();
            // Reset _lcd_scanline back to original
            _lcd_scanline = OVERLAY_START_Y + win_height;
            // ...and finally potentially skip entries if parallax_row, if overlay went past them
            // (...TODO!)
        }
    }
    if(_lcd_scanline == 0)
        y = draw_scroll_y;
    else
        y = _lcd_scanline;
    move_bkg(parallax_row->scx, y);
    _lcd_scanline = parallax_row->next_y + 1;
    //
    if(parallax_row->next_y == 0)
        parallax_row = parallax_rows;
    else
        parallax_row++;
}


//typedef struct parallax_row_t {
//    UBYTE scx;      // x scroll position for current slice
//    UBYTE next_y;   // y position of next LYC
//    INT8  shift;    // shift of scroll position within the world
//    UBYTE start_tile;
//    UBYTE tile_height;
//    UBYTE shadow_scx;
//} parallax_row_t;
