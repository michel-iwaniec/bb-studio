#pragma bank 255

#include "vm_actor.h"

#include <gbdk/metasprites.h>

#include "actor.h"
#include "game_time.h"
#include "data_manager.h"
#include "scroll.h"
#include "math.h"
#include "macro.h"

BANKREF(VM_ACTOR)

#define EMOTE_TOTAL_FRAMES         60
#define MOVE_INACTIVE              0
#define MOVE_ALLOW_H               1
#define MOVE_ALLOW_V               2
#define MOVE_DIR_H                 4
#define MOVE_DIR_V                 8
#define MOVE_ACTIVE_H              16
#define MOVE_ACTIVE_V              32
#define MOVE_NEEDED_H              64
#define MOVE_NEEDED_V              128
#define MOVE_H                     (MOVE_ALLOW_H | MOVE_NEEDED_H)
#define MOVE_V                     (MOVE_ALLOW_V | MOVE_NEEDED_V)
#define TILE_FRACTION_MASK         0b1111111
#define ONE_TILE_DISTANCE          128


typedef struct act_move_to_t {
    INT16 ID;
    INT16 X, Y;
    UBYTE ATTR;
} act_move_to_t;

typedef struct act_set_pos_t {
    INT16 ID;
    INT16 X, Y;
} act_set_pos_t;

typedef struct act_set_frame_t {
    INT16 ID;
    INT16 FRAME;
} act_set_frame_t;

typedef struct gbs_farptr_t {
    INT16 BANK;
    const void * DATA;
} gbs_farptr_t;

// Optimized versions for vm_actor_move_to
inline UBYTE bb_intersects_opt(uint16_t a_l, uint16_t a_r, uint16_t a_t, uint16_t a_b, uint16_t a_x, uint16_t a_y, bounding_box_16_t *bb_b, upoint16_t *offset_b) {
    if ((offset_b->x + bb_b->left   > a_r) ||
        (offset_b->x + bb_b->right  < a_l)) return FALSE;
    if ((offset_b->y + bb_b->top    > a_b) ||
        (offset_b->y + bb_b->bottom < a_t)) return FALSE;
    return TRUE;
}

inline actor_t *actor_overlapping_bb_opt(bounding_box_16_t *bb, uint16_t pos_x, uint16_t pos_y, actor_t *ignore) {
    actor_t *actor = &PLAYER;

    uint16_t r = pos_x + bb->right;
    uint16_t l = pos_x + bb->left;
    uint16_t b = pos_y + bb->bottom;
    uint16_t t = pos_y + bb->top;

    while (actor) {
        if (actor == ignore || !actor->collision_enabled) {
            actor = actor->prev;
            continue;
        };

        if (bb_intersects_opt(l, r, t, b, pos_x, pos_y, &actor->bounds_x16, &actor->pos)) {
            return actor;
        }

        actor = actor->prev;
    }

    return NULL;
}

//
static UBYTE check_collision_LEFT_tc(UBYTE tx1, UBYTE tx2, UBYTE ty1, UBYTE ty2) {
    UBYTE tt;
    while (tx1 != tx2) {
        tt = ty1;
        while (tt != ty2) {
            if (tile_at(tx1, tt) & COLLISION_RIGHT) {
                //return ((tx1 + 1) << 7) - bounds_x16->left;
                return tx1;
            }
            tt++;
        }
        tx1--;
    }
    return 255;
}

inline UWORD check_collision_LEFT(UWORD start_x, UWORD start_y, bounding_box_16_t *bounds_x16, UWORD end_pos) {
    UBYTE tx1;
    tx1 = check_collision_LEFT_tc(  (start_x + bounds_x16->left) >> 7,
                                    ((end_pos + bounds_x16->left) >> 7) - 1,
                                    ((start_y + bounds_x16->top) >> 7),
                                    ((start_y + bounds_x16->bottom) >> 7) + 1);
    return tx1 == 255 ? end_pos : ((tx1 + 1) << 7) - bounds_x16->left;
}

//
static UBYTE check_collision_RIGHT_tc(UBYTE tx1, UBYTE tx2, UBYTE ty1, UBYTE ty2) {
    UBYTE  tt;
    while (tx1 != tx2) {
        tt = ty1;
        while (tt != ty2) {
            if (tile_at(tx1, tt) & COLLISION_LEFT) {
                //return (tx1 << 7) - (bounds_x16->right + 16);
                return tx1;
            }
            tt++;
        }
        tx1++;
    }
    return 255;
}

inline UWORD check_collision_RIGHT(UWORD start_x, UWORD start_y, bounding_box_16_t *bounds_x16, UWORD end_pos) {
    UBYTE tx1;
    tx1 = check_collision_RIGHT_tc( (start_x + bounds_x16->right) >> 7,
                                    ((end_pos + bounds_x16->right) >> 7) + 1,
                                    ((start_y + bounds_x16->top) >> 7),
                                    ((start_y + bounds_x16->bottom) >> 7) + 1);
    return tx1 == 255 ? end_pos : (tx1 << 7) - (bounds_x16->right + 16);
}

//
static UWORD check_collision_UP_tc(UBYTE tx1, UBYTE tx2, UBYTE ty1, UBYTE ty2) {
    UBYTE tt;
    while (ty1 != ty2) {
        tt = tx1;
        while (tt != tx2) {
            if (tile_at(tt, ty1) & COLLISION_BOTTOM) {
                //return ((ty1 + 1) << 7) - (bounds_x16->top);
                return ty1;
            }
            tt++;
        }
        ty1--;
    }
    return 255;
}

inline UWORD check_collision_UP(UWORD start_x, UWORD start_y, bounding_box_16_t *bounds_x16, UWORD end_pos) {
    UBYTE ty1;
    ty1 = check_collision_UP_tc(    ((start_x + bounds_x16->left) >> 7),
                                    ((start_x + bounds_x16->right) >> 7) + 1,
                                    ((start_y + bounds_x16->top) >> 7),
                                    ((end_pos + bounds_x16->top) >> 7) - 1);
    return ty1 == 255 ? end_pos : ((ty1 + 1) << 7) - (bounds_x16->top);
}

//
static UWORD check_collision_DOWN_tc(UBYTE tx1, UBYTE tx2, UBYTE ty1, UBYTE ty2) {
    UBYTE tt;
    while (ty1 != ty2) {
        tt = tx1;
        while (tt != tx2) {
            if (tile_at(tt, ty1) & COLLISION_TOP) {
                //return ((ty1) << 7) - (bounds_x16->bottom + 16);
                return ty1;
            }
            tt++;
        }
        ty1++;
    }
    return 255;
}

inline UWORD check_collision_DOWN(UWORD start_x, UWORD start_y, bounding_box_16_t *bounds_x16, UWORD end_pos) {
    UBYTE ty1;
    ty1 = check_collision_DOWN_tc(  ((start_x + bounds_x16->left) >> 7),
                                    ((start_x + bounds_x16->right) >> 7) + 1,
                                    ((start_y + bounds_x16->bottom) >> 7),
                                    ((end_pos + bounds_x16->bottom) >> 7) + 1);
    return ty1 == 255 ? end_pos : ((ty1) << 7) - (bounds_x16->bottom + 16);
}

static void vm_actor_move_to_impl(SCRIPT_CTX * THIS, INT16 idx) {
    actor_t *actor;
    static direction_e new_dir = DIR_DOWN;
    static INT16 params_X;
    static INT16 params_Y;
    static UINT8 params_ATTR;
    static UINT16 actor_pos_x;
    static UINT16 actor_pos_y;
    static UINT8 actor_move_speed;
    static UINT8 flags;
    static direction_e new_dir_flipped;

    // indicate waitable state of context
    THIS->waitable = 1;

    act_move_to_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);
    
    params_X = params->X;
    params_Y = params->Y;
    params_ATTR = params->ATTR;
    actor_pos_x = actor->pos.x;
    actor_pos_y = actor->pos.y;
    actor_move_speed = actor->move_speed;
    flags = THIS->flags;
    
    if (flags == 0) {
        actor->movement_interrupt = FALSE;

        // Switch to moving animation frames
        actor_set_anim_moving(actor);

        // Snap to nearest pixel before moving
        actor_pos_x = actor_pos_x & 0xFFF0;
        actor_pos_y = actor_pos_y & 0xFFF0;

        if (CHK_FLAG(params_ATTR, ACTOR_ATTR_DIAGONAL)) {
            SET_FLAG(flags, MOVE_ALLOW_H | MOVE_ALLOW_V);
        } if (CHK_FLAG(params_ATTR, ACTOR_ATTR_H_FIRST)) {
            SET_FLAG(flags, MOVE_ALLOW_H);
        } else {
            SET_FLAG(flags, MOVE_ALLOW_V);
        }

        // Check for collisions in path
        if (CHK_FLAG(params_ATTR, ACTOR_ATTR_CHECK_COLL)) {
            if (CHK_FLAG(params_ATTR, ACTOR_ATTR_H_FIRST)) {
                // Check for horizontal collision
                if (actor_pos_x != params_X) {
                    if(actor_pos_x > params_X) {
                        params_X = check_collision_LEFT(actor_pos_x, actor_pos_y, &actor->bounds_x16, params_X);
                    } else {
                        params_X = check_collision_RIGHT(actor_pos_x, actor_pos_y, &actor->bounds_x16, params_X);
                    }
                }
                // Check for vertical collision
                if (actor_pos_y != params_Y) {
                    if(actor_pos_y > params_Y) {
                        params_Y = check_collision_UP(params_X, actor_pos_y, &actor->bounds_x16, params_Y);
                    } else {
                        params_Y = check_collision_DOWN(params_X, actor_pos_y, &actor->bounds_x16, params_Y);
                    }
                }
            } else {
                // Check for vertical collision
                if (actor_pos_y != params_Y) {
                    if(actor_pos_y > params_Y) {
                        params_Y = check_collision_UP(actor_pos_x, actor_pos_y, &actor->bounds_x16, params_Y);
                    } else {
                        params_Y = check_collision_DOWN(actor_pos_x, actor_pos_y, &actor->bounds_x16, params_Y);
                    }
                }
                // Check for horizontal collision
                if (actor_pos_x != params_X) {
                    if(actor_pos_x > params_X) {
                        params_X = check_collision_LEFT(actor_pos_x, params_Y, &actor->bounds_x16, params_X);
                    } else {
                        params_X = check_collision_RIGHT(actor_pos_x, params_Y, &actor->bounds_x16, params_X);
                    }
                }
            }
        }

        // Actor already at destination
        if ((actor_pos_x != params_X)) {
            SET_FLAG(flags, MOVE_NEEDED_H);
        } else {
            SET_FLAG(flags, MOVE_ALLOW_V);
        }
        if (actor_pos_y != params_Y) {
            SET_FLAG(flags, MOVE_NEEDED_V);
        } else {
            SET_FLAG(flags, MOVE_ALLOW_H);
        }

        // Initialise movement directions
        if (actor_pos_x > params_X) {
            // Move left
            SET_FLAG(flags, MOVE_DIR_H);
        }
        if (actor_pos_y > params_Y) {
            // Move up
            SET_FLAG(flags, MOVE_DIR_V);
        }
    }

    // Interrupt actor movement
    if (actor->movement_interrupt) {
        // Set new X destination to next tile
        if ((actor_pos_x < params_X) && (actor_pos_x & TILE_FRACTION_MASK)) {   // Bitmask to check for non-grid-aligned position
            params_X = (actor_pos_x & ~TILE_FRACTION_MASK) + ONE_TILE_DISTANCE;  // If moving in positive direction, round up to next tile
        } else {
            params_X = actor_pos_x  & ~TILE_FRACTION_MASK;                       // Otherwise, round down
        }
        // Set new Y destination to next tile
        if ((actor_pos_y < params_Y) && (actor_pos_y & TILE_FRACTION_MASK)) {
            params_Y = (actor_pos_y & ~TILE_FRACTION_MASK) + ONE_TILE_DISTANCE;
        } else {
            params_Y = actor_pos_y  & ~TILE_FRACTION_MASK;
        }
        actor->movement_interrupt = FALSE;
    }

    // Move in X Axis
    if (CHK_FLAG(flags, MOVE_H) == MOVE_H) {
        // Get hoizontal direction from flags
        new_dir = CHK_FLAG(flags, MOVE_DIR_H) ? DIR_LEFT : DIR_RIGHT;

        // Move actor
        //point_translate_dir(&actor->pos, new_dir, actor->move_speed);
        if(new_dir == DIR_RIGHT)
            actor_pos_x += actor_move_speed;
        else if(new_dir == DIR_LEFT)
            actor_pos_x -= actor_move_speed;
        else if(new_dir == DIR_DOWN)
            actor_pos_y += actor_move_speed;
        else if(new_dir == DIR_UP)
            actor_pos_y -= actor_move_speed;
    
        // Check for actor collision
        //actor->pos.x = actor_pos_x;
        //actor->pos.y = actor_pos_y;
        if (CHK_FLAG(params_ATTR, ACTOR_ATTR_CHECK_COLL) && actor_overlapping_bb_opt(&actor->bounds_x16, actor_pos_x, actor_pos_y, actor)) {
            //point_translate_dir(&actor->pos, FLIPPED_DIR(new_dir), actor->move_speed);
            new_dir_flipped = FLIPPED_DIR(new_dir);
            if(new_dir_flipped == DIR_RIGHT)
                actor_pos_x += actor_move_speed;
            else if(new_dir_flipped == DIR_LEFT)
                actor_pos_x -= actor_move_speed;
            else if(new_dir_flipped == DIR_DOWN)
                actor_pos_y += actor_move_speed;
            else if(new_dir_flipped == DIR_UP)
                actor_pos_y -= actor_move_speed;
            flags = 0;
            actor_set_anim_idle(actor);
            goto exit_function;
        }

        // If first frame moving in this direction update actor direction
        if (!CHK_FLAG(flags, MOVE_ACTIVE_H)) {
            SET_FLAG(flags, MOVE_ACTIVE_H);
            actor_set_dir(actor, new_dir, TRUE);
        }

        // Check if overshot destination
        if (
            (new_dir == DIR_LEFT && (actor_pos_x <= params_X)) || // Overshot left
            (new_dir == DIR_RIGHT && (actor_pos_x >= params_X))   // Overshot right
        ) {
            // Reached Horizontal Destination
            actor_pos_x = params_X;
            SET_FLAG(flags, MOVE_ALLOW_V);
            CLR_FLAG(flags, MOVE_H);
        }
    }

    // Move in Y Axis
    if (CHK_FLAG(flags, MOVE_V) == MOVE_V) {
        // Get vertical direction from flags
        new_dir = CHK_FLAG(flags, MOVE_DIR_V) ? DIR_UP : DIR_DOWN;

        // Move actor
        //point_translate_dir(&actor->pos, new_dir, actor->move_speed);
        if(new_dir == DIR_RIGHT)
            actor_pos_x += actor_move_speed;
        else if(new_dir == DIR_LEFT)
            actor_pos_x -= actor_move_speed;
        else if(new_dir == DIR_DOWN)
            actor_pos_y += actor_move_speed;
        else if(new_dir == DIR_UP)
            actor_pos_y -= actor_move_speed;

        // Check for actor collision
        //actor->pos.x = actor_pos_x;
        //actor->pos.y = actor_pos_y;
        if (CHK_FLAG(params_ATTR, ACTOR_ATTR_CHECK_COLL) && actor_overlapping_bb_opt(&actor->bounds_x16, actor_pos_x, actor_pos_y, actor)) {
            //point_translate_dir(&actor->pos, FLIPPED_DIR(new_dir), actor->move_speed);
            new_dir_flipped = FLIPPED_DIR(new_dir);
            if(new_dir_flipped == DIR_RIGHT)
                actor_pos_x += actor_move_speed;
            else if(new_dir_flipped == DIR_LEFT)
                actor_pos_x -= actor_move_speed;
            else if(new_dir_flipped == DIR_DOWN)
                actor_pos_y += actor_move_speed;
            else if(new_dir_flipped == DIR_UP)
                actor_pos_y -= actor_move_speed;
            flags = 0;
            actor_set_anim_idle(actor);
            goto exit_function;
        }

        // If first frame moving in this direction update actor direction
        if (!CHK_FLAG(flags, MOVE_ACTIVE_V)) {
            SET_FLAG(flags, MOVE_ACTIVE_V);
            actor_set_dir(actor, new_dir, TRUE);
        }

        // Check if overshot destination
        if (
            (new_dir == DIR_UP && (actor_pos_y <= params_Y)) || // Overshot above
            (new_dir == DIR_DOWN &&  (actor_pos_y >= params_Y)) // Overshot below
         ) {
            actor_pos_y = params_Y;
            SET_FLAG(flags, MOVE_ALLOW_H);
            CLR_FLAG(flags, MOVE_V);
        }
    }

    // Actor reached destination
    if (!CHK_FLAG(flags, MOVE_NEEDED_H | MOVE_NEEDED_V)) {
        flags = MOVE_INACTIVE;
        actor_set_anim_idle(actor);
        goto exit_function;
    }

    THIS->PC -= (INSTRUCTION_SIZE + sizeof(idx));
exit_function:
    actor->pos.x = actor_pos_x;
    actor->pos.y = actor_pos_y;
    params->X = params_X;
    params->Y = params_Y;
    THIS->flags = flags;
    return;
}

void vm_actor_move_to(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT
{
    vm_actor_move_to_impl(THIS, idx);
}

void vm_actor_move_cancel(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor_t * actor = actors + *n_actor;

    actor->movement_interrupt = TRUE;
}

void vm_actor_activate(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor_t * actor = actors + *n_actor;
    if (actor == &PLAYER) {
        actor->hidden = FALSE;
    } else {
        actor->disabled = FALSE;
        activate_actor(actor);
    }
}

void vm_actor_deactivate(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor_t * actor = actors + *n_actor;
    if (actor == &PLAYER) {
        actor->hidden = TRUE;
    } else {
        actor->disabled = TRUE;
        deactivate_actor(actor);
    }
}

void vm_actor_begin_update(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    actor_t *actor;

    act_set_pos_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    if ((actor->script_update.bank) && (actor->hscript_update & SCRIPT_TERMINATED)) {
        script_execute(actor->script_update.bank, actor->script_update.ptr, &(actor->hscript_update), 0);
    }
}

void vm_actor_terminate_update(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    actor_t *actor;

    act_set_pos_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    if ((actor->hscript_update & SCRIPT_TERMINATED) == 0) {
        script_terminate(actor->hscript_update);
    }
}

void vm_actor_set_dir(SCRIPT_CTX * THIS, INT16 idx, direction_e dir) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor_set_dir(actors + *n_actor, dir, FALSE);
}

void vm_actor_set_anim(SCRIPT_CTX * THIS, INT16 idx, INT16 idx_anim) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    UBYTE * n_anim = VM_REF_TO_PTR(idx_anim);
    actor_set_anim(actors + *n_actor, *n_anim);
}

void vm_actor_set_pos(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    actor_t *actor;

    act_set_pos_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    actor->pos.x = params->X;
    actor->pos.y = params->Y;
}

void vm_actor_get_pos(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    actor_t *actor;

    act_set_pos_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    params->X = actor->pos.x;
    params->Y = actor->pos.y;
}

void vm_actor_get_dir(SCRIPT_CTX * THIS, INT16 idx, INT16 dest) OLDCALL BANKED REENTRANT {
    UWORD * A;
    actor_t *actor;

    act_set_pos_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    if (dest < 0) A = THIS->stack_ptr + dest; else A = script_memory + dest;
    *A = actor->dir;
}

void vm_actor_get_angle(SCRIPT_CTX * THIS, INT16 idx, INT16 dest) OLDCALL BANKED REENTRANT {
    UWORD * A;
    actor_t *actor;

    act_set_pos_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    if (dest < 0) A = THIS->stack_ptr + dest; else A = script_memory + dest;
    *A = dir_angle_lookup[actor->dir];
}

void vm_actor_emote(SCRIPT_CTX * THIS, INT16 idx, UBYTE emote_tiles_bank, const unsigned char *emote_tiles) OLDCALL BANKED REENTRANT {

    // on first call load emote sprite
    if (THIS->flags == 0) {
        UBYTE * n_actor = VM_REF_TO_PTR(idx);
        THIS->flags = 1;
        emote_actor = actors + *n_actor;
        emote_timer = 1;
        load_emote(emote_tiles, emote_tiles_bank);
    }

    if (emote_timer == EMOTE_TOTAL_FRAMES) {
        // Reset ctx flags
        THIS->flags = 0;
        emote_actor = NULL;
    } else {
        THIS->waitable = 1;
        emote_timer++;
        THIS->PC -= (INSTRUCTION_SIZE + sizeof(idx) + sizeof(emote_tiles_bank) + sizeof(emote_tiles));
    }
}

void vm_actor_set_bounds(SCRIPT_CTX * THIS, INT16 idx, BYTE left, BYTE right, BYTE top, BYTE bottom) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor_t * actor = actors + *n_actor;
    actor->bounds.left = left;
    actor->bounds.right = right;
    actor->bounds.top = top;
    actor->bounds.bottom = bottom;
    //
    actor_update_bounds_x16(actor);
}

void vm_actor_set_spritesheet(SCRIPT_CTX * THIS, INT16 idx, UBYTE spritesheet_bank, const spritesheet_t *spritesheet) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor_t * actor = actors + *n_actor;
    load_sprite(actor->base_tile, spritesheet, spritesheet_bank);
    actor->sprite.bank = spritesheet_bank;
    actor->sprite.ptr = (void *)spritesheet;
    load_animations(spritesheet, spritesheet_bank, ANIM_SET_DEFAULT, actor->animations);
    load_bounds(spritesheet, spritesheet_bank, &actor->bounds);
    actor_reset_anim(actor);
}

void vm_actor_replace_tile(SCRIPT_CTX * THIS, INT16 idx, UBYTE target_tile, UBYTE tileset_bank, const tileset_t * tileset, UBYTE start_tile, UBYTE length) OLDCALL BANKED REENTRANT {
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor_t * actor = actors + *n_actor;
    SetBankedSpriteData(actor->base_tile + target_tile, length, tileset->tiles + (start_tile << 4), tileset_bank);
}

void vm_actor_set_anim_tick(SCRIPT_CTX * THIS, INT16 idx, UBYTE tick) OLDCALL BANKED REENTRANT {
    actor_t *actor;
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor = actors + *n_actor;
    actor->anim_tick = tick;
}

void vm_actor_set_move_speed(SCRIPT_CTX * THIS, INT16 idx, UBYTE speed) OLDCALL BANKED REENTRANT {
    actor_t *actor;
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor = actors + *n_actor;
    actor->move_speed = speed;
}

void vm_actor_set_anim_frame(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    actor_t *actor;

    act_set_frame_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    actor_set_frame_offset(actor, params->FRAME);
}

void vm_actor_get_anim_frame(SCRIPT_CTX * THIS, INT16 idx) OLDCALL BANKED REENTRANT {
    actor_t *actor;

    act_set_frame_t * params = VM_REF_TO_PTR(idx);
    actor = actors + (UBYTE)(params->ID);

    params->FRAME = actor_get_frame_offset(actor);
}

void vm_actor_set_anim_set(SCRIPT_CTX * THIS, INT16 idx, UWORD offset) OLDCALL BANKED REENTRANT {
    actor_t *actor;
    UBYTE * n_actor = VM_REF_TO_PTR(idx);
    actor = actors + *n_actor;
    load_animations(actor->sprite.ptr, actor->sprite.bank, offset, actor->animations);
    actor_reset_anim(actor);
}

void vm_actor_set_spritesheet_by_ref(SCRIPT_CTX * THIS, INT16 idxA, INT16 idxB) OLDCALL BANKED REENTRANT {
    actor_t *actor;
    UBYTE * n_actor = VM_REF_TO_PTR(idxA);
    actor = actors + *n_actor;

    gbs_farptr_t * params = VM_REF_TO_PTR(idxB);
    UBYTE spritesheet_bank = (UBYTE)(params->BANK);
    const spritesheet_t *spritesheet = params->DATA;

    load_sprite(actor->base_tile, spritesheet, spritesheet_bank);
    actor->sprite.bank = spritesheet_bank;
    actor->sprite.ptr = (void *)spritesheet;
    load_animations(spritesheet, spritesheet_bank, ANIM_SET_DEFAULT, actor->animations);
    load_bounds(spritesheet, spritesheet_bank, &actor->bounds);
    actor_reset_anim(actor);
}

void vm_actor_set_flags(SCRIPT_CTX * THIS, INT16 idx, UBYTE flags, UBYTE mask) OLDCALL BANKED REENTRANT {
    actor_t * actor = actors + *(UBYTE *)VM_REF_TO_PTR(idx);

    if (mask & ACTOR_FLAG_PINNED)      actor->pinned            = (flags & ACTOR_FLAG_PINNED);
    if (mask & ACTOR_FLAG_HIDDEN)      actor->hidden            = (flags & ACTOR_FLAG_HIDDEN);
    if (mask & ACTOR_FLAG_ANIM_NOLOOP) actor->anim_noloop       = (flags & ACTOR_FLAG_ANIM_NOLOOP);
    if (mask & ACTOR_FLAG_COLLISION)   actor->collision_enabled = (flags & ACTOR_FLAG_COLLISION);
    if (mask & ACTOR_FLAG_PERSISTENT)  actor->persistent        = (flags & ACTOR_FLAG_PERSISTENT);
}