#pragma bank 255

#include <string.h>

#include "system.h"
#include "vm.h"
#include "data_manager.h"
#include "linked_list.h"
#include "actor.h"
#include "projectiles.h"
#include "scroll.h"
#include "trigger.h"
#include "camera.h"
#include "ui.h"
#include "palette.h"
#include "data/spritesheet_none.h"
#include "data/data_bootstrap.h"

#define ALLOC_BKG_TILES_TOWARDS_SPR

#define EMOTE_SPRITE_SIZE       4

far_ptr_t current_scene;

UBYTE image_bank;
unsigned char* image_ptr;

UBYTE image_attr_bank;
unsigned char* image_attr_ptr;
unsigned char* image_attr_nes_ptr;

UBYTE collision_bank;
unsigned char* collision_ptr;

extern UBYTE collision_row_addr_lo[128];
extern UBYTE collision_row_addr_hi[128];
//extern UBYTE image_row_offs_lo[128];
//extern UBYTE image_row_offs_hi[128];
extern UBYTE image_row_addr_lo[128];
extern UBYTE image_row_addr_hi[128];

UBYTE scene_scroll_x_adjust;
int16_t scene_scroll_y_adjust;
int16_t scene_sprite_px_offset_x_compensate;
int16_t scene_sprite_px_offset_y_compensate;
UBYTE image_tile_width;
UBYTE image_tile_height;
UINT16 image_attr_size;
UINT16 image_width;
UINT16 image_height;
UBYTE sprites_len;
UBYTE actors_len;
UBYTE projectiles_len;
UBYTE player_sprite_len;
scene_type_e scene_type;
LCD_isr_e scene_LCD_type;
UBYTE scene_SHOW_LC;

const far_ptr_t spritesheet_none_far = TO_FAR_PTR_T(spritesheet_none);

scene_stack_item_t scene_stack[SCENE_STACK_SIZE];
scene_stack_item_t * scene_stack_ptr;

UBYTE scene_sprites_base_tiles[MAX_SCENE_SPRITES];

void load_init(void) BANKED {
    actors_len = 0;
    player_sprite_len = 0;
    scene_stack_ptr = scene_stack;
}

// gbdk-nes: Helper function to try and find a suitable blank tile
UBYTE FindBlankTile(const tileset_t* tiles, UBYTE bank)
{
    UBYTE n_tiles = ReadBankedUWORD(&(tiles->n_tiles), bank);
    UBYTE blank_idx = find_blank_tile(tiles->tiles, bank, n_tiles, 0x00, 0x00);
    return blank_idx;
}

static volatile uint8_t* CFG_REG = (volatile uint8_t*)0x8000;

// gbdk-nes: Reset UI attributes to palette 3
const uint8_t data_ff[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
void clear_ui_attributes(void) BANKED REENTRANT {
    for(uint8_t i = 0; i < 4; i++)
    {
        *CFG_REG = i;
        for(uint8_t j = 252; j != 0; j++)
        {
            set_bkg_data(j, 1, data_ff);
        }
        *CFG_REG = 0;
    }
}

void load_bkg_tileset(const tileset_t* tiles, UBYTE bank) BANKED REENTRANT {
    if ((!bank) || (!tiles)) return;

    UWORD n_tiles = ReadBankedUWORD(&(tiles->n_tiles), bank);

    // load first background chunk, align to zero tile
    UBYTE * data = tiles->tiles;
    if (n_tiles < 128) {
        if ((UBYTE)n_tiles) SetBankedBkgData(0, n_tiles, data, bank);
        return;
    }
    SetBankedBkgData(0, 128, data, bank);
    n_tiles -= 128; data += 128 * 16;

    // load second background chunk
    if (n_tiles < 128) {
        if (n_tiles < 65) {
            #ifdef ALLOC_BKG_TILES_TOWARDS_SPR
                // new allocation style, align to 192-th tile
                if ((UBYTE)n_tiles) SetBankedBkgData(192 - n_tiles, n_tiles, data, bank);
            #else
                // old allocation style, align to 128-th tile
                if ((UBYTE)n_tiles) SetBankedBkgData(128, n_tiles, data, bank);
            #endif
        } else {
            // if greater than 64 allow overflow into UI, align to 128-th tile
            if ((UBYTE)n_tiles) SetBankedBkgData(128, n_tiles, data, bank);
        }
        return;
    }
    SetBankedBkgData(128, 128, data, bank);
    n_tiles -= 128; data += 128 * 16;

    // if more than 256 - then it's a logo image, load rest to CHR banks 1..3
    for(uint8_t i = 1; i < 4; i++)
    { 
        *CFG_REG = i;
        if(n_tiles >= 256)
        {
            SetBankedBkgData(0, 128, data, bank);
            data += 128 * 16;
            SetBankedBkgData(128, 128, data, bank);
            data += 128 * 16;
            n_tiles -= 256;
        }
        else if(n_tiles >= 0)
        {
            SetBankedBkgData(0, n_tiles, data, bank);
        }
        *CFG_REG = 0;
    }
}

void load_background(const background_t* background, UBYTE bank) BANKED REENTRANT {
    background_t bkg;
    DISPLAY_OFF;
    
    MemcpyBanked(&bkg, background, sizeof(bkg), bank);

    UBYTE blank_tile = FindBlankTile(bkg.tileset.ptr, bkg.tileset.bank);

    // Clear screen (TODO: Do it less hacky...)
    // Fill the screen background with a single tile pattern
    fill_bkg_rect(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, blank_tile);

    image_bank = bkg.tilemap.bank;
    image_ptr = bkg.tilemap.ptr;

    image_attr_bank = bkg.cgb_tilemap_attr.bank;
    image_attr_ptr = bkg.cgb_tilemap_attr.ptr;
    image_attr_nes_ptr = bkg.cgb_tilemap_attr.ptr; //bkg.cgb_tilemap_attr_nes.ptr;

    image_tile_width = bkg.width;
    image_tile_height = bkg.height;
    image_width = image_tile_width * 8;
    image_attr_size = bkg.attr_nes_width * bkg.attr_nes_height;

    if(image_width > (UINT16)SCREENWIDTH)
    {
        scroll_x_max = image_width - ((UINT16)SCREENWIDTH);
    }
    else
    {
        // image smaller than screen width - just set to SCREENWIDTH.
        scroll_x_max = SCREENWIDTH;
    }
    image_height = image_tile_height * 8;
    if(image_height > (UINT16)SCREENHEIGHT)
    {
        scroll_y_max = image_height - ((UINT16)SCREENHEIGHT);
    }
    else
    {
        // image smaller than screen height - just set to SCREENHEIGHT.
        scroll_y_max = SCREENHEIGHT;
    }

    clear_ui_attributes();

    load_bkg_tileset(bkg.tileset.ptr, bkg.tileset.bank);
#ifdef CGB
    if ((_is_CGB) && (bkg.cgb_tileset.ptr)) {
        // TODO: gbdk-nes attributes
        //VBK_REG = 1;
        //load_bkg_tileset(bkg.cgb_tileset.ptr, bkg.cgb_tileset.bank);
        //VBK_REG = 0;
    }
#endif
    DISPLAY_ON;
    // Clear all attribute planes to zero
    for(UBYTE y = 0; y < DEVICE_SCREEN_HEIGHT; y++)
    {
        for(UBYTE x = 0; x < DEVICE_SCREEN_WIDTH; x++)
        {
            set_attribute_xy(x, y, 0);
        }
    }
}

inline UBYTE load_sprite_tileset(UBYTE base_tile, const tileset_t * tileset, UBYTE bank) {
    UBYTE n_tiles = ReadBankedUBYTE(&(tileset->n_tiles), bank);
    if (n_tiles) SetBankedSpriteData(base_tile, n_tiles, tileset->tiles, bank);
    return n_tiles;
}

UBYTE load_sprite(UBYTE sprite_offset, const spritesheet_t * sprite, UBYTE bank) BANKED {
    far_ptr_t data;
    ReadBankedFarPtr(&data, (void *)&sprite->tileset, bank);
    UBYTE n_tiles = load_sprite_tileset(sprite_offset, data.ptr, data.bank);
#ifdef CGB
    if (_is_CGB) {
        ReadBankedFarPtr(&data, (void *)&sprite->cgb_tileset, bank);
        if (data.ptr) {
            //VBK_REG = 1;
            UBYTE n_cgb_tiles = load_sprite_tileset(sprite_offset, data.ptr, data.bank);
            //VBK_REG = 0;
            if (n_cgb_tiles > n_tiles) return n_cgb_tiles;
        }
    }
#endif
    return n_tiles;
}

void load_animations(const spritesheet_t *sprite, UBYTE bank, UWORD animation_set, animation_t * res_animations) NONBANKED  REENTRANT {
    UBYTE _save = CURRENT_BANK;
    SWITCH_ROM(bank);
    memcpy(res_animations, sprite->animations + sprite->animations_lookup[animation_set], sizeof(animation_t) * 8);
    SWITCH_ROM(_save);
}

void load_bounds(const spritesheet_t *sprite, UBYTE bank, bounding_box_t * res_bounds) BANKED REENTRANT {
    MemcpyBanked(res_bounds, &sprite->bounds, sizeof(sprite->bounds), bank);
}

UBYTE do_load_palette(palette_entry_t * dest, const palette_t * palette, UBYTE bank) BANKED REENTRANT {
    UBYTE mask = ReadBankedUBYTE(&palette->mask, bank);
    palette_entry_t * sour = palette->cgb_palette;
    for (UBYTE i = mask; (i); i >>= 1, dest++) {
        if ((i & 1) == 0) continue;
        MemcpyBanked(dest, sour, sizeof(palette_entry_t), bank);
        sour++;
    }
    return mask;
}

inline void load_bkg_palette(const palette_t * palette, UBYTE bank) {
    UBYTE mask = do_load_palette(BkgPalette, palette, bank);
    DMG_palette[0] = ReadBankedUBYTE(palette->palette, bank);
#ifdef SGB
    if (_is_SGB) {
        UBYTE sgb_palettes = SGB_PALETTES_NONE;
        if (mask & 0b00110000) sgb_palettes |= SGB_PALETTES_01;
        if (mask & 0b11000000) sgb_palettes |= SGB_PALETTES_23;
        SGBTransferPalettes(sgb_palettes);
    }
#endif
}

inline void load_sprite_palette(const palette_t * palette, UBYTE bank) {
    do_load_palette(SprPalette, palette, bank);
    UWORD data = ReadBankedUWORD(palette->palette, bank);
    DMG_palette[1] = (UBYTE)data;
    DMG_palette[2] = (UBYTE)(data >> 8);
}

UBYTE load_scene(const scene_t * scene, UBYTE bank, UBYTE init_data) BANKED REENTRANT {
    UBYTE i;
    static scene_t scn;

    MemcpyBanked(&scn, scene, sizeof(scn), bank);

    current_scene.bank  = bank;
    current_scene.ptr   = (void *)scene;

    // Load scene
    scene_type      = scn.type;
    actors_len      = MIN(scn.n_actors + 1,     MAX_ACTORS);
    triggers_len    = MIN(scn.n_triggers,       MAX_TRIGGERS);
    projectiles_len = MIN(scn.n_projectiles,    MAX_PROJECTILE_DEFS);
    sprites_len     = MIN(scn.n_sprites,        MAX_SCENE_SPRITES);

    collision_bank  = scn.collisions.bank;
    collision_ptr   = scn.collisions.ptr;

    // Load UI tiles, they may be overwritten by the following load_background()
    ui_load_tiles();

    // Load background + tiles
    load_background(scn.background.ptr, scn.background.bank);

    // gbdk-nes optimization: Create lookup-table for tile_at(..., ...) calls to avoid multiplies
    for(i = 0; i < 128; i++)
    {
        unsigned char* c = collision_ptr + i * image_tile_width;
        collision_row_addr_lo[i] = (((UWORD)c) & 0xFF);
        collision_row_addr_hi[i] = ((((UWORD)c) >> 8) & 0xFF);
        //
        c = image_ptr + i * image_tile_width;
        image_row_addr_lo[i] = (((UWORD)c) & 0xFF);
        image_row_addr_hi[i] = ((((UWORD)c) >> 8) & 0xFF);
        //
        //c = i * image_tile_width;
        //image_row_offs_lo[i] = (((UWORD)c) & 0xFF);
        //image_row_offs_hi[i] = ((((UWORD)c) >> 8) & 0xFF);
    }

    load_bkg_palette(scn.palette.ptr, scn.palette.bank);
    load_sprite_palette(scn.sprite_palette.ptr, scn.sprite_palette.bank);

    // Copy parallax settings
    memcpy(&parallax_rows, &scn.parallax_rows, sizeof(parallax_rows));
    if (scn.parallax_rows[0].next_y == 0) {
        scene_LCD_type = (scene_type == SCENE_TYPE_LOGO) ? LCD_fullscreen : LCD_simple;
    } else {
        scene_LCD_type = LCD_parallax;
    }

    if (scene_type != SCENE_TYPE_LOGO) {
        // Load player
        PLAYER.sprite = scn.player_sprite;
        UBYTE n_loaded = load_sprite(PLAYER.base_tile = 0, scn.player_sprite.ptr, scn.player_sprite.bank);
        allocated_sprite_tiles = (n_loaded > scn.reserve_tiles) ? n_loaded : scn.reserve_tiles;
        load_animations(scn.player_sprite.ptr, scn.player_sprite.bank, ANIM_SET_DEFAULT, PLAYER.animations);
        load_bounds(scn.player_sprite.ptr, scn.player_sprite.bank, &PLAYER.bounds);
    } else {
        // no player on logo, but still some little amount of actors may be present
        PLAYER.base_tile = allocated_sprite_tiles = 0x68;
        PLAYER.sprite = spritesheet_none_far;
        memset(PLAYER.animations, 0, sizeof(PLAYER.animations));
    }

    // Load sprites
    if (sprites_len != 0) {
        far_ptr_t * scene_sprite_ptrs = scn.sprites.ptr;
        far_ptr_t tmp_ptr;
        for (i = 0; i != sprites_len; i++) {
            if (i == MAX_SCENE_SPRITES) break;
            ReadBankedFarPtr(&tmp_ptr, (UBYTE *)scene_sprite_ptrs, scn.sprites.bank);
            scene_sprites_base_tiles[i] = allocated_sprite_tiles;
            allocated_sprite_tiles += load_sprite(allocated_sprite_tiles, tmp_ptr.ptr, tmp_ptr.bank);
            scene_sprite_ptrs++;
        }
    }

    if (init_data) {
        camera_reset();

        // Copy scene player hit scripts to player actor
        memcpy(&PLAYER.script, &scn.script_p_hit1, sizeof(far_ptr_t));

        player_moving = FALSE;

        // Load actors
        actors_active_head = NULL;
        actors_inactive_head = NULL;

        // Add player to inactive, then activate
        PLAYER.active = FALSE;
        actor_update_bounds_x16(&PLAYER);
        actors_active_tail = &PLAYER;
        DL_PUSH_HEAD(actors_inactive_head, actors_active_tail);
        activate_actor(&PLAYER);

        // Add other actors, activate pinned
        if (actors_len != 0) {
            actor_t * actor = actors + 1;
            MemcpyBanked(actor, scn.actors.ptr, sizeof(actor_t) * (actors_len - 1), scn.actors.bank);
            for (i = actors_len - 1; i != 0; i--, actor++) {
                actor_update_bounds_x16(actor);
                if (actor->reserve_tiles) {
                    // exclusive sprites allocated separately to avoid overwriting if modified
                    actor->base_tile = allocated_sprite_tiles;
                    UBYTE n_loaded = load_sprite(allocated_sprite_tiles, actor->sprite.ptr, actor->sprite.bank);
                    allocated_sprite_tiles += (n_loaded > actor->reserve_tiles) ? n_loaded : actor->reserve_tiles;
                } else {
                    // resolve and set base_tile for each actor
                    UBYTE idx = IndexOfFarPtr(scn.sprites.ptr, scn.sprites.bank, sprites_len, &actor->sprite);
                    actor->base_tile = (idx < sprites_len) ? scene_sprites_base_tiles[idx] : 0;
                }
                load_animations((void *)actor->sprite.ptr, actor->sprite.bank, ANIM_SET_DEFAULT, actor->animations);
                // add to inactive list by default
                actor->active = FALSE;
                DL_PUSH_HEAD(actors_inactive_head, actor);

                // activate if the actor is pinned or persistent
                if ((actor->pinned) || (actor->persistent)) activate_actor(actor);
            }
        }

    } else {
        // reload sprite data for the unique actors
        if (actors_len != 0) {
            actor_t * actor = actors + 1;
            for (i = actors_len - 1; i != 0; i--, actor++) {
                // exclusive sprites allocated separately to avoid overwriting if modified
                if (actor->reserve_tiles) load_sprite(actor->base_tile, actor->sprite.ptr, actor->sprite.bank);
            }
        }
        // set actors idle
        actor_t *actor = actors_active_head;
        while (actor) {
            actor_set_anim_idle(actor);
            actor = actor->next;
        }
    }

    // Init and Load projectiles
    projectiles_init();
    if (projectiles_len  != 0) {
        projectile_def_t * projectile_def = projectile_defs;
        MemcpyBanked(projectile_def, scn.projectiles.ptr, sizeof(projectile_def_t) * projectiles_len, scn.projectiles.bank);
        for (i = projectiles_len; i != 0; i--, projectile_def++) {
            projectile_def_update_bounds_x16(projectile_def);
            // resolve and set base_tile for each projectile
            UBYTE idx = IndexOfFarPtr(scn.sprites.ptr, scn.sprites.bank, sprites_len, &projectile_def->sprite);
            projectile_def->base_tile = (idx < sprites_len) ? scene_sprites_base_tiles[idx] : 0;
        }
    }

    // Load triggers
    if (triggers_len != 0) {
        MemcpyBanked(&triggers, scn.triggers.ptr, sizeof(trigger_t) * triggers_len, scn.triggers.bank);
    }

    scroll_reset();
    trigger_reset();

    emote_actor = NULL;
    
    // gbdk-nes: For non-scrolling backgrounds, there's no need to hide leftmost column
    scene_SHOW_LC &= ~(PPUMASK_SHOW_BG_LC | PPUMASK_SHOW_SPR_LC);
    scene_SHOW_LC = (image_tile_width <= 32) ? (PPUMASK_SHOW_BG_LC | PPUMASK_SHOW_SPR_LC) : 0;
    // gbdk-nes: For non-scrolling backgrounds, there's no need to offset scroll coordinates
    scene_scroll_x_adjust = (image_tile_width <= 32) ? 0 : (uint8_t)-8;
    scene_sprite_px_offset_x_compensate = 
        (image_tile_width <= 32) ?   
        DEVICE_SPRITE_PX_OFFSET_X_COMPENSATE :
        DEVICE_SPRITE_PX_OFFSET_X_COMPENSATE + 8;
    scene_scroll_y_adjust = (image_tile_height <= 30) ? 0 : -4;
    scene_sprite_px_offset_y_compensate = DEVICE_SPRITE_PX_OFFSET_Y_COMPENSATE - scene_scroll_y_adjust;

    if ((init_data) && (scn.script_init.ptr != NULL)) {
        return (script_execute(scn.script_init.bank, scn.script_init.ptr, 0, 0) != 0);
    }
    return FALSE;
}

void load_player(void) BANKED {
    PLAYER.pos.x = start_scene_x;
    PLAYER.pos.y = start_scene_y;
    PLAYER.dir = start_scene_dir;
    PLAYER.move_speed = start_player_move_speed;
    PLAYER.anim_tick = start_player_anim_tick;
    PLAYER.frame = 0;
    PLAYER.frame_start = 0;
    PLAYER.frame_end = 2;
    PLAYER.pinned = FALSE;
    PLAYER.collision_group = COLLISION_GROUP_PLAYER;
    PLAYER.collision_enabled = TRUE;
}

void load_emote(const unsigned char *tiles, UBYTE bank) BANKED {
    SetBankedSpriteData(allocated_sprite_tiles, EMOTE_SPRITE_SIZE, tiles + 0, bank);
}
