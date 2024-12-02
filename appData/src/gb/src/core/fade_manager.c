#pragma bank 255

#include <gbdk/platform.h>

#include "compat.h"
#include "system.h"
#include "fade_manager.h"
#include "palette.h"

#define FADED_OUT_FRAME 5
#define FADED_IN_FRAME 0

UBYTE fade_running;
UBYTE fade_frames_per_step;
UBYTE fade_timer;
UBYTE fade_style = 0;

const UBYTE fade_speeds[] = {0x0, 0x1, 0x3, 0x7, 0xF, 0x1F, 0x3F};

static UBYTE fade_frame;
static FADE_DIRECTION fade_direction;

#ifdef CGB

void CGBFadeToWhiteStep(const palette_entry_t * pal, UBYTE reg, UBYTE step) OLDCALL BANKED REENTRANT {
    reg; step;
    const uint8_t* pal8 = (const uint8_t*)pal;
    uint8_t i, j, k;
    UBYTE r = 0;
    for(i = 0; i < 4; i++)
    {
        for(k = 0; k < 4; k++)
        {
            UBYTE ph = pal8[(i << 2) + k] & 0x0F;
            UBYTE pl = pal8[(i << 2) + k] & 0x30;
            if(ph >= 0x0E)
            {
                pl = 0x10;
                ph = 0x0D;
            }
            for(j = 0; j < step; j++)
            {            
                if(ph >= 0 && ph < 0x0D)
                {
                    if(pl != 0x30)
                    {
                        pl = (pl+0x10);
                    }
                    else
                    {
                        pl = 0x30;
                        ph = 0x00;
                    }
                }
                else if(ph == 0x0D)
                {
                    if(pl == 0x30)
                        ph = 0x00;
                    else
                        pl = pl+0x10;
                }
            }
            if(reg == BCPS_REG_ADDR)
                set_bkg_palette_entry(i, k, pl | ph);
            else if(reg == OCPS_REG_ADDR && k != 0)
                set_sprite_palette_entry(i, k, pl | ph);
        }
    }
}

void CGBFadeToBlackStep(const palette_entry_t * pal, UBYTE reg, UBYTE step) OLDCALL BANKED REENTRANT {
    reg; step;
    const uint8_t* pal8 = (const uint8_t*)pal;
    uint8_t i, j, k;
    UBYTE r = 0;
    for(i = 0; i < 4; i++)
    {
        for(k = 0; k < 4; k++)
        {
            UBYTE ph = pal8[(i << 2) + k] & 0x0F;
            UBYTE pl = pal8[(i << 2) + k] & 0x30;
            if(ph >= 0x0E)
            {
                pl = 0x10;
                ph = 0x0D;
            }
            for(j = 0; j < step; j++)
            {            
                if(ph >= 0 && ph < 0x0D)
                {
                    if(pl != 0x00)
                    {
                        pl = (pl-0x10);
                    }
                    else
                    {
                        pl = 0x10;
                        ph = 0x0D;
                    }
                }
                else if(ph >= 0x0D)
                {
                    if(ph > 0x0D)
                    {
                        pl = 0x10;
                        ph = 0x0D;
                    }
                    else if(pl <= 0x10)
                    {
                        pl = 0x10;
                        ph = 0x0D;
                    }
                    else
                    {
                        pl = pl-0x10;
                    }
                }
            }
            if(reg == BCPS_REG_ADDR)
                set_bkg_palette_entry(i, k, pl | ph);
            else if(reg == OCPS_REG_ADDR && k != 0)
                set_sprite_palette_entry(i, k, pl | ph);
        }
    }
}

void ApplyPaletteChangeColor(UBYTE index) {
    if (fade_style) {
        CGBFadeToBlackStep(BkgPalette, BCPS_REG_ADDR, index);
        CGBFadeToBlackStep(SprPalette, OCPS_REG_ADDR, index);
    } else {
        CGBFadeToWhiteStep(BkgPalette, BCPS_REG_ADDR, index);
        CGBFadeToWhiteStep(SprPalette, OCPS_REG_ADDR, index);
    }
}
#endif

#if defined(GAMEBOY)
UBYTE DMGFadeToWhiteStep(UBYTE step, UBYTE pal) NAKED {
    pal; step;
__asm
#if defined(__SDCC) && defined(NINTENDO)
        or      A
        jr      Z, 0$

        ld      D, A
1$:
        ld      H, #4
2$:
        ld      A, E
        and     #3
        jr      Z, 3$
        dec     A
3$:
        srl     A
        rr      L
        srl     A
        rr      L

        srl     E
        srl     E

        dec     H
        jr      NZ, 2$

        ld      E, L

        dec     D
        jr      NZ, 1$
0$:
        ld      A, E
#endif
        ret
__endasm;
}
#elif defined(NINTENDO_NES)
UBYTE DMGFadeToWhiteStep(UBYTE step, UBYTE pal) {
    pal; step;

    uint8_t i, j;
    UBYTE r = 0;
    for(j = 0; j < step; j++)
    {
        for(i = 0; i < 4; i++)
        {
            UBYTE p = pal & 0x3;
            r = (r >> 2);
            r |= ((p != 0x00) ? (p-1) : 0x00) << 6;
            pal = (pal >> 2);
        }
        pal = r;
    }

    return pal;
}
#endif

#if defined(GAMEBOY)
UBYTE DMGFadeToBlackStep(UBYTE step, UBYTE pal) NAKED {
    pal; step;
__asm
#if defined(__SDCC) && defined(NINTENDO)
        or      A
        jr      Z, 0$

        ld      D, A
1$:
        ld      H, #4
2$:
        ld      A, E
        and     #3
        cp      #3
        jr      Z, 3$
        inc     A
3$:
        srl     A
        rr      L
        srl     A
        rr      L

        srl     E
        srl     E

        dec     H
        jr      NZ, 2$

        ld      E, L

        dec     D
        jr      NZ, 1$
0$:
        ld      A, E
#endif
        ret
__endasm;
}
#elif defined(NINTENDO_NES)
UBYTE DMGFadeToBlackStep(UBYTE step, UBYTE pal) {
    pal; step;

    uint8_t i, j;
    UBYTE r = 0;
    for(j = 0; j < step; j++)
    {
        for(i = 0; i < 4; i++)
        {
            UBYTE p = pal & 0x3;
            r = (r >> 2);
            r |= ((p != 0x03) ? (p+1) : 0x03) << 6;
            pal = (pal >> 2);
        }
        pal = r;
    }

    return pal;
}
#endif

const uint8_t dmg_pal_lookup[] = { 0x30, 0x10, 0x00, 0x1D };

inline void SetDMGPaletteBGP(uint8_t p)
{
#if defined(GAMEBOY)
    BGP_REG = p;
#elif defined(NINTENDO_NES)
    set_bkg_palette_entry(0, 0, dmg_pal_lookup[(p >> 0) & 0x3]);
    set_bkg_palette_entry(0, 1, dmg_pal_lookup[(p >> 2) & 0x3]);
    set_bkg_palette_entry(0, 2, dmg_pal_lookup[(p >> 4) & 0x3]);
    set_bkg_palette_entry(0, 3, dmg_pal_lookup[(p >> 6) & 0x3]);
#else
    //
#endif
}

inline void SetDMGPaletteOBP0(uint8_t p)
{
#if defined(GAMEBOY)
    OBP0_REG = p;
#elif defined(NINTENDO_NES)
    //set_sprite_palette_entry(0, 0, dmg_pal_lookup[(p >> 0) & 0x3]);
    set_sprite_palette_entry(0, 1, dmg_pal_lookup[(p >> 2) & 0x3]);
    set_sprite_palette_entry(0, 2, dmg_pal_lookup[(p >> 4) & 0x3]);
    set_sprite_palette_entry(0, 3, dmg_pal_lookup[(p >> 6) & 0x3]);
#else
    //
#endif
}

inline void SetDMGPaletteOBP1(uint8_t p)
{
#if defined(GAMEBOY)
    OBP1_REG = p;
#elif defined(NINTENDO_NES)
    //set_sprite_palette_entry(1, 0, dmg_pal_lookup[(p >> 0) & 0x3]);
    set_sprite_palette_entry(1, 1, dmg_pal_lookup[(p >> 2) & 0x3]);
    set_sprite_palette_entry(1, 2, dmg_pal_lookup[(p >> 4) & 0x3]);
    set_sprite_palette_entry(1, 3, dmg_pal_lookup[(p >> 6) & 0x3]);
#else
    //
#endif
}

void ApplyPaletteChangeDMG(UBYTE index) {
    if (index > 4) index = 4;
    if (!fade_style) {
        SetDMGPaletteBGP(DMGFadeToWhiteStep(index, DMG_palette[0]));
        SetDMGPaletteOBP0(DMGFadeToWhiteStep(index, DMG_palette[1]));
        SetDMGPaletteOBP1(DMGFadeToWhiteStep(index, DMG_palette[2]));
    } else {
        SetDMGPaletteBGP(DMGFadeToBlackStep(index, DMG_palette[0]));
        SetDMGPaletteOBP0(DMGFadeToBlackStep(index, DMG_palette[1]));
        SetDMGPaletteOBP1(DMGFadeToBlackStep(index, DMG_palette[2]));
    }
}

void fade_init(void) BANKED {
    fade_frames_per_step = fade_speeds[2];
    fade_timer = FADED_OUT_FRAME;
    fade_running = FALSE;
#ifdef CGB
    if (_is_CGB) {
        ApplyPaletteChangeColor(fade_timer);
        return;
    }
#endif
    ApplyPaletteChangeDMG(FADED_OUT_FRAME);
}

void fade_in(void) BANKED {
    if (fade_timer == FADED_IN_FRAME) {
#ifdef CGB
        if (_is_CGB) {
            ApplyPaletteChangeColor(FADED_IN_FRAME);
            return;
        }
#endif
        ApplyPaletteChangeDMG(FADED_IN_FRAME);
        return;
    }
    fade_frame = 0;
    fade_direction = FADE_IN;
    fade_running = TRUE;
    fade_timer = FADED_OUT_FRAME;
#ifdef CGB
    if (_is_CGB) {
        ApplyPaletteChangeColor(FADED_OUT_FRAME);
        return;
    }
#endif
    ApplyPaletteChangeDMG(FADED_OUT_FRAME);
}

void fade_out(void) BANKED {
    if (fade_timer == FADED_OUT_FRAME) {
#ifdef CGB
        if (_is_CGB) {
            ApplyPaletteChangeColor(FADED_OUT_FRAME);
            return;
        }
#endif
        ApplyPaletteChangeDMG(FADED_OUT_FRAME);
        return;
    }
    fade_frame = 0;
    fade_direction = FADE_OUT;
    fade_running = TRUE;
    fade_timer = FADED_IN_FRAME;
#ifdef CGB
    if (_is_CGB) {
        ApplyPaletteChangeColor(fade_timer);
        return;
    }
#endif
        ApplyPaletteChangeDMG(FADED_IN_FRAME);
}

void fade_update(void) BANKED REENTRANT {
    if (fade_running) {
        if ((fade_frame++ & fade_frames_per_step) == 0) {
            if (fade_direction == FADE_IN) {
                if (fade_timer > FADED_IN_FRAME) fade_timer--;
                if (fade_timer == FADED_IN_FRAME) fade_running = FALSE;
            } else {
                if (fade_timer < FADED_OUT_FRAME) fade_timer++;
                if (fade_timer == FADED_OUT_FRAME) fade_running = FALSE;
            }
#ifdef CGB
            if (_is_CGB) {
                ApplyPaletteChangeColor(fade_timer);
                return;
            }
#endif
            ApplyPaletteChangeDMG(fade_timer);
        }
    }
}

void fade_applypalettechange(void) BANKED {
#ifdef CGB
    if (_is_CGB) {
        ApplyPaletteChangeColor(fade_timer);
        return;
    }
#endif
    ApplyPaletteChangeDMG(fade_timer);
}

void fade_setspeed(UBYTE speed) BANKED {
    fade_frames_per_step = fade_speeds[speed];
}

void fade_in_modal(void) BANKED {
    fade_in();
    while (fade_isfading()) {
        wait_vbl_done();
        fade_update();
    }
}

void fade_out_modal(void) BANKED {
    fade_out();
    while (fade_isfading()) {
        wait_vbl_done();
        fade_update();
    }
}
