#pragma bank 255

#include "scroll.h"

#include <string.h>

#include "system.h"
#include "actor.h"
#include "camera.h"
#include "data_manager.h"
#include "game_time.h"
#include "math.h"
#include "fade_manager.h"
#include "parallax.h"
#include "palette.h"

extern UBYTE scene_scroll_x_adjust;

// put submap of a large map to screen
/*
void set_bkg_submap_attributes_gbcfmt(UINT8 x, UINT8 y, UINT8 w, UINT8 h, const unsigned char *map, UINT8 map_w) NONBANKED REENTRANT
{
    uint16_t offs = (uint16_t)y*((uint16_t)map_w) + (uint16_t)x;
    UINT8 ym = y % 30;
    for(UINT8 i = 0; i < h; i++)
    {
        UINT8 xm = x & 31;
        for(UINT8 j = 0; j < w; j++)
        {
            UINT8 a = map[offs+j];
            set_bkg_attribute_xy(xm, ym, a);
            xm = (xm + 1) & 31;
        }        
        ym = (ym + 1);
        if(ym >= 30)
            ym -= 30;
        offs += map_w;
    }
}
*/

extern uint8_t attribute_shadow_offset;

void set_bkg_submap_attributes_8x8(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *attributes, uint8_t map_w) NONBANKED
{
    //attribute_shadow_offset = ((y & 1) << 7) | ((x & 1) << 6);
    if(w == 1)
    {
        // Special case: Render just one column - update only 2 planes
        attributes += (x & 1) ? image_attr_size : 0;
        attribute_shadow_offset = ((x & 1) << 6);
        // First process even y...
        set_bkg_submap_attributes(x,
                                  (y & 1) ? (y+1) : y, //y, // & 0xFE,
                                  w,
                                  (y & 1) ? (h-1) : h, //h,
                                  attributes,
                                  map_w);
        // ...then process odd y.
        attribute_shadow_offset |= 0x80;
        attributes += (image_attr_size << 1);
        set_bkg_submap_attributes(x,
                                  y, // | 1,
                                  w,
                                  h,
                                  attributes,
                                  map_w);
        attribute_shadow_offset = 0;
    }
    else if(h == 1)
    {
        // Special case: Render just one row - update only 2 planes
        attributes += (y & 1) ? (image_attr_size << 1) : 0;
        attribute_shadow_offset = ((y & 1) << 7);
        // First process even x...        
        set_bkg_submap_attributes((x & 1) ? (x+1) : x, //x,
                                  y,
                                  (x & 1) ? (w-1) : w, // w,
                                  h,
                                  attributes,
                                  map_w);
        // ...then process odd x.
        attribute_shadow_offset |= 0x40;
        attributes += image_attr_size;
        set_bkg_submap_attributes(x,
                                  y,
                                  (w & 1) ? (w & 0xFE) : w,
                                  h,
                                  attributes,
                                  map_w);
        attribute_shadow_offset = 0;    
    }
    else
    {
        // Generalized case: Must update all 4 planes
        /*
        attribute_shadow_offset = 0;
        for(uint8_t i = 0; i < 4; i++) {
            set_bkg_submap_attributes(x,
                                      y,
                                      w,
                                      h,
                                      attributes,
                                      map_w);
            attributes += image_attr_size;
            attribute_shadow_offset += 0x40;
        }
        */
        
        attribute_shadow_offset = 0;
        // Top-left
        set_bkg_submap_attributes((x & 1) ? (x+1) : x, // x
                                  y,
                                  (x & 1) ? (w-1) : w, // w,
                                  h,
                                  attributes,
                                  map_w);
        attributes += image_attr_size;
        attribute_shadow_offset += 0x40;
        // Top-right
        set_bkg_submap_attributes(x,
                                  y,
                                  (w & 1) ? (w & 0xFE) : w,
                                  h,
                                  attributes,
                                  map_w);
        attributes += image_attr_size;
        attribute_shadow_offset += 0x40;
        // Bottom-left
        set_bkg_submap_attributes((x & 1) ? (x+1) : x, // x,
                                  y,
                                  (x & 1) ? (w-1) : w, // w,
                                  h,
                                  attributes,
                                  map_w);
        attributes += image_attr_size;
        attribute_shadow_offset += 0x40;
        // Bottom-right
        set_bkg_submap_attributes(x,
                                  y,
                                  (w & 1) ? (w & 0xFE) : w,
                                  h,
                                  attributes,
                                  map_w);
        attributes += image_attr_size;
        attribute_shadow_offset += 0x40;

        //attribute_shadow_offset = 0;
    }
}


#define set_bkg_submap_attributes_ set_bkg_submap_attributes_8x8

void scroll_queue_row(UBYTE x, UBYTE y);
void scroll_queue_col(UBYTE x, UBYTE y);
void scroll_load_pending_row(void);
void scroll_load_pending_col(void);
void scroll_load_row(UBYTE x, UBYTE y);
void scroll_load_col(UBYTE x, UBYTE y, UBYTE height);
void scroll_render_rows(INT16 scroll_x, INT16 scroll_y, BYTE row_offset, BYTE n_rows);
UBYTE scroll_viewport(parallax_row_t * port);

INT16 scroll_x;
INT16 scroll_y;
INT16 draw_scroll_x;
INT16 draw_scroll_y;
UINT16 scroll_x_max;
UINT16 scroll_y_max;
BYTE scroll_offset_x;
BYTE scroll_offset_y;
UBYTE pending_h_x, pending_h_y;
UBYTE pending_h_i;
UBYTE pending_w_x, pending_w_y;
UBYTE pending_w_i;
INT16 current_row, new_row;
INT16 current_col, new_col;
UBYTE needs_full_update;

void scroll_init(void) BANKED {
    draw_scroll_x   = 0;
    draw_scroll_y   = 0;
    scroll_x_max    = 0;
    scroll_y_max    = 0;
    scroll_offset_x = 0;
    scroll_offset_y = 0;
    scroll_reset();
}

void scroll_reset(void) BANKED {
    pending_w_i     = 0;
    pending_h_i     = 0;
    scroll_x        = 0x7FFF;
    scroll_y        = 0x7FFF;

    game_time       = 0; // was in scroll_render_rows() - that is insane, here is not the best place either 
}

void scroll_update(void) BANKED {
    INT16 x, y;
    INT16 sw, sh;
    UBYTE render = FALSE;

    // prevent redundant scrolling on NES when map is smaller than screen
    sw = (image_width > (UINT16)SCREENWIDTH) ? (SCREENWIDTH >> 1) : SCREENWIDTH;
    sh = (image_height > (UINT16)SCREENHEIGHT) ? (SCREENHEIGHT >> 1) : SCREENHEIGHT;

    x = (camera_x >> 4) - sw; //(SCREENWIDTH >> 1);
    y = (camera_y >> 4) - sh; //(SCREENHEIGHT >> 1);

    if (x & 0x8000u) {  // check for negative signed bit
        x = 0u;
    } else if (x > scroll_x_max) {
        x = scroll_x_max;
    }
    if (y & 0x8000u) {
        y = 0u;
    } else if (y > scroll_y_max) {
        y = scroll_y_max;
    }

    current_col = scroll_x >> 3;
    current_row = scroll_y >> 3;
    new_col = x >> 3;
    new_row = y >> 3;

    scroll_x = x;
    scroll_y = y;
    draw_scroll_x = x + scroll_offset_x;
    draw_scroll_y = y + scroll_offset_y;

    if (scroll_viewport(parallax_rows)) return;
    if (scroll_viewport(parallax_rows + 1)) return;
    scroll_viewport(parallax_rows + 2);
}

UBYTE scroll_viewport(parallax_row_t * port) {
    if (port->next_y) {
        // one of upper parallax slices
        UINT16 shift_scroll_x;
        if (port->shift == 127) {
            shift_scroll_x = 0;
        } else if (port->shift < 0) {
            shift_scroll_x = draw_scroll_x << (-port->shift);
        } else {
            shift_scroll_x = draw_scroll_x >> port->shift;
        }

        port->shadow_scx = shift_scroll_x + scene_scroll_x_adjust;        
        UBYTE shift_col = shift_scroll_x >> 3;

        // If column is +/- 1 just render next column
        if (current_col == new_col - 1) {
            // Render right column
            UBYTE x = shift_col - SCREEN_PAD_LEFT + SCREEN_TILE_REFRES_W - 1;
            scroll_load_col(x, port->start_tile, port->tile_height);
        } else if (current_col == new_col + 1) {
            // Render left column
            UBYTE x = MAX(0, shift_col - SCREEN_PAD_LEFT);
            scroll_load_col(x, port->start_tile, port->tile_height);
        } else if (current_col != new_col) {
            // If column differs by more than 1 render entire viewport
            scroll_render_rows(shift_scroll_x, 0, port->start_tile, port->tile_height);
        }  
        return FALSE;   
    } else {
        // last parallax slice OR no parallax
        port->shadow_scx = draw_scroll_x + scene_scroll_x_adjust;

        // If column is +/- 1 just render next column
        UBYTE x;
        UBYTE full_y = 255;
        //needs_full_update = FALSE;
        if (!needs_full_update && current_col == new_col - 1) {
            // Queue right column
            x = new_col - SCREEN_PAD_LEFT + SCREEN_TILE_REFRES_W - 1;
            UBYTE y = MAX(0, MAX((new_row - SCREEN_PAD_TOP), port->start_tile));
            full_y = MAX(0, (new_row - SCREEN_PAD_TOP));
            scroll_queue_col(x, y);
        } else if (!needs_full_update && current_col == new_col + 1) {
            // Queue left column
            x = MAX(0, new_col - SCREEN_PAD_LEFT);
            UBYTE y = MAX(0, MAX((new_row - SCREEN_PAD_TOP), port->start_tile));
            full_y = MAX(0, (new_row - SCREEN_PAD_TOP));
            scroll_queue_col(x, y);
        } else if (current_col != new_col || needs_full_update) {
            // If column differs by more than 1 render entire screen
            scroll_render_rows(draw_scroll_x, draw_scroll_y, ((scene_LCD_type == LCD_parallax) ? port->start_tile : -SCREEN_PAD_TOP), SCREEN_TILE_REFRES_H);
            needs_full_update = FALSE;
            return TRUE;
        }
        if(full_y != 255)
            activate_actors_in_col_inline(x, full_y);

        // If row is +/- 1 just render next row
        if (!needs_full_update && current_row == new_row - 1) {
            // Queue bottom row
            UBYTE x = MAX(0, new_col - SCREEN_PAD_LEFT);
            UBYTE y = new_row - SCREEN_PAD_TOP + SCREEN_TILE_REFRES_H - 1;
            scroll_queue_row(x, y);
            activate_actors_in_row(x, y);
        } else if (!needs_full_update && current_row == new_row + 1) {
            // Queue top row
            UBYTE x = MAX(0, new_col - SCREEN_PAD_LEFT);
            UBYTE y = MAX(port->start_tile, new_row - SCREEN_PAD_TOP);
            scroll_queue_row(x, y);
            activate_actors_in_row(x, y);
        } else if (current_row != new_row || needs_full_update) {
            // If row differs by more than 1 render entire screen
            scroll_render_rows(draw_scroll_x, draw_scroll_y, ((scene_LCD_type == LCD_parallax) ? port->start_tile : -SCREEN_PAD_TOP), SCREEN_TILE_REFRES_H);
            needs_full_update = FALSE;
            return TRUE;
        }

        if (IS_FRAME_2) {
            if (pending_h_i) scroll_load_pending_col();
            if (pending_w_i) scroll_load_pending_row();
        }

        return TRUE;
    }
}

void scroll_repaint(void) BANKED {
    scroll_reset();
    scroll_update();
}

void scroll_render_rows(INT16 scroll_x, INT16 scroll_y, BYTE row_offset, BYTE n_rows) {
    // Clear pending rows/ columns
    pending_w_i = 0;
    pending_h_i = 0;

    UBYTE x = MAX(0, (scroll_x >> 3) - SCREEN_PAD_LEFT);
    UBYTE y = MAX(0, (scroll_y >> 3) + row_offset);

    for (BYTE i = 0; i != n_rows && y != image_tile_height; ++i, y++) {
        scroll_load_row(x, y);
        activate_actors_in_row(x, y);
    }
}

void scroll_queue_row(UBYTE x, UBYTE y) {
    while (pending_w_i) {
        // If previous row wasn't fully rendered
        // render it now before starting next row        
        scroll_load_pending_row();
    }

    // Don't queue rows past image height
    if (y >= image_tile_height) {
        return;
    }

    pending_w_x = x;
    pending_w_y = y;
    pending_w_i = SCREEN_TILE_REFRES_W;
}

void scroll_queue_col(UBYTE x, UBYTE y) {
    while (pending_h_i) {
        // If previous column wasn't fully rendered
        // render it now before starting next column
        scroll_load_pending_col();
    }

    pending_h_x = x;
    pending_h_y = y;
    pending_h_i = MIN(SCREEN_TILE_REFRES_H, image_tile_height - y);
}

/* Update pending (up to 5) rows */
void scroll_load_pending_row(void) NONBANKED {
    UINT8 _save = CURRENT_BANK;
    UBYTE width = MIN(pending_w_i, PENDING_BATCH_SIZE);
#if 1 // DEBUGHACK: Skip submap updates
#ifdef CGB
    if (_is_CGB) {  // Color Row Load
        SWITCH_ROM(image_attr_bank);
        if(image_tile_width)
            set_bkg_submap_attributes_(pending_w_x, pending_w_y, width, 1, image_attr_nes_ptr, image_tile_width);
    }
#endif
    // DMG Row Load
    SWITCH_ROM(image_bank);
    if(image_tile_width)
        set_bkg_submap(pending_w_x, pending_w_y, width, 1, image_ptr, image_tile_width);
#endif
    pending_w_x += width;
    pending_w_i -= width;

    SWITCH_ROM(_save);
}


void scroll_load_row(UBYTE x, UBYTE y) NONBANKED {
#if 1 // DEBUGHACK: Skip submap updates
    UINT8 _save = CURRENT_BANK;
#ifdef CGB
    if (_is_CGB) {  // Color Column Load
        SWITCH_ROM(image_attr_bank);
        if(image_tile_width)
            //set_bkg_submap_attributes_(x, y, SCREEN_TILE_REFRES_W, 1, image_attr_nes_ptr, image_tile_width);
            set_bkg_submap_attributes_(x, y, SCREEN_TILE_REFRES_W, 1, image_attr_nes_ptr, image_tile_width);

    }
#endif
    // DMG Row Load
    SWITCH_ROM(image_bank);
    if(image_tile_width)
        //set_bkg_submap(x, y, MIN(SCREEN_TILE_REFRES_W, image_tile_width), 1, image_ptr, image_tile_width);
        set_bkg_submap(x, y, MIN(SCREEN_TILE_REFRES_W, image_tile_width), 1, image_ptr, image_tile_width);

    SWITCH_ROM(_save);
#endif
}

void scroll_load_col(UBYTE x, UBYTE y, UBYTE height) NONBANKED {
#if 1 // DEBUGHACK: Skip submap updates
    UINT8 _save = CURRENT_BANK;
#ifdef CGB
    if (_is_CGB) {  // Color Column Load
        SWITCH_ROM(image_attr_bank);
        if(image_tile_width)
            set_bkg_submap_attributes_(x, y, 1, height, image_attr_nes_ptr, image_tile_width);
    }
#endif
    // DMG Column Load
    unsigned char* map = image_ptr + image_tile_width * y + x;
    SWITCH_ROM(image_bank);
    if(image_tile_width)
        set_bkg_submap(x, y, 1, height, image_ptr, image_tile_width);
    SWITCH_ROM(_save);
#endif
}

void scroll_load_pending_col(void) NONBANKED {
    UINT8 _save = CURRENT_BANK;
    UBYTE height = MIN(pending_h_i, PENDING_BATCH_SIZE);
#if 1 // DEBUGHACK: Skip submap updates
    SWITCH_ROM(image_bank);
#ifdef CGB
    if (_is_CGB) {  // Color Column Load
        SWITCH_ROM(image_attr_bank);
        if(image_tile_width)
            set_bkg_submap_attributes_(pending_h_x, pending_h_y, 1, height, image_attr_nes_ptr, image_tile_width);
    }
#endif
    // DMG Column Load
    SWITCH_ROM(image_bank);
    if(image_tile_width)
        set_bkg_submap(pending_h_x, pending_h_y, 1, height, image_ptr, image_tile_width);
#endif
    pending_h_y += height;
    pending_h_i -= height;

    SWITCH_ROM(_save);
}
