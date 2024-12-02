#ifndef ACTOR_H
#define ACTOR_H

#include <gbdk/platform.h>
#include "bankdata.h"
#include "gbs_types.h"
#include "linked_list.h"
#include "vm.h"
#include "scroll.h"

#ifdef NINTENDO_NES
    #define DEVICE_SPRITE_PX_OFFSET_X_COMPENSATE -8
    #define DEVICE_SPRITE_PX_OFFSET_Y_COMPENSATE -17
#else
    #define DEVICE_SPRITE_PX_OFFSET_X_COMPENSATE 0
    #define DEVICE_SPRITE_PX_OFFSET_Y_COMPENSATE 0
#endif

#define MAX_ACTORS            21
#define MAX_ACTORS_ACTIVE     12

#define PLAYER                actors[0]
#define ON_8PX_GRID(A)        ( MOD_8((A).x >> 4) == 0 &&  MOD_8((A).y >> 4) == 0)
#define ON_16PX_GRID(A)       (MOD_16((A).x >> 4) == 0 && MOD_16((A).y >> 4) == 8)

#define PLAYER_HURT_IFRAMES   20

#define ANIM_JUMP_LEFT        0
#define ANIM_JUMP_RIGHT       2
#define ANIM_CLIMB            6

#define ANIM_CURSOR           0
#define ANIM_CURSOR_HOVER     1

#define ANIM_SET_DEFAULT      0

BANKREF_EXTERN(ACTOR)

typedef enum {
  CHECK_DIR_LEFT = 1,
  CHECK_DIR_RIGHT,
  CHECK_DIR_UP,
  CHECK_DIR_DOWN,
} col_check_dir_e;

extern actor_t actors[MAX_ACTORS];
extern actor_t * actors_active_head;
extern actor_t * actors_active_tail;
extern actor_t * actors_inactive_head;
extern UBYTE player_moving;
extern actor_t * player_collision_actor;
extern actor_t * emote_actor;
extern UBYTE emote_timer;

extern UBYTE allocated_sprite_tiles;
extern UBYTE allocated_hardware_sprites;

void actor_set_frames(actor_t *actor, UBYTE frame_start, UBYTE frame_end) NONBANKED;
inline void actor_set_anim(actor_t *actor, UBYTE anim) {
    actor->animation = anim;
    actor_set_frames(actor, actor->animations[anim].start, actor->animations[anim].end + 1);
}

void actors_init(void) BANKED REENTRANT;
void actors_update(void) NONBANKED;
void deactivate_actor(actor_t *actor) BANKED REENTRANT;
void activate_actor(actor_t *actor) BANKED REENTRANT;

void actor_set_frame_offset(actor_t *actor, UBYTE frame_offset) BANKED REENTRANT;
// gbdk-nes: Changed to inline as simple function
//UBYTE actor_get_frame_offset(actor_t *actor) BANKED REENTRANT;
inline UBYTE actor_get_frame_offset(actor_t *actor) {
    return actor->frame - actor->frame_start;
}

actor_t *actor_at_tile(UBYTE tx, UBYTE ty, UBYTE inc_noclip) BANKED REENTRANT;
actor_t *actor_in_front_of_player(UBYTE grid_size, UBYTE inc_noclip) BANKED REENTRANT;
actor_t *actor_overlapping_player(UBYTE inc_noclip) BANKED;
actor_t *actor_overlapping_bb(bounding_box_16_t *bb, upoint16_t *offset, actor_t *ignore, UBYTE inc_noclip) BANKED REENTRANT;
// gbdk-nes: Changed to inline as simple 1-line wrapper function 
//void actor_set_anim_idle(actor_t *actor) BANKED REENTRANT;
inline void actor_set_anim_idle(actor_t *actor) {
    actor_set_anim(actor, actor->dir);
}
// gbdk-nes: Changed to inline as simple 1-line wrapper function 
//void actor_set_anim_moving(actor_t *actor) BANKED REENTRANT;
inline void actor_set_anim_moving(actor_t *actor) {
    actor_set_anim(actor, actor->dir + N_DIRECTIONS);
}
// gbdk-nes: Change to inline function for speed (costly as duplicated in many places!)
//void actor_set_dir(actor_t *actor, direction_e dir, UBYTE moving) BANKED REENTRANT;
inline void actor_set_dir(actor_t *actor, direction_e dir, UBYTE moving) {
    actor->dir = dir;
    if (moving) {
        actor_set_anim(actor, dir + N_DIRECTIONS);
    } else {
        actor_set_anim(actor, dir);
    }
}

inline void actor_reset_anim(actor_t *actor) {
    actor_set_frames(actor, actor->animations[actor->animation].start, actor->animations[actor->animation].end + 1);
}
inline void actor_stop_anim(actor_t *actor) {
    actor->frame_start = actor->frame;
    actor->frame_end = actor->frame + 1;
}
inline void player_register_collision_with(actor_t *actor) {
    player_collision_actor = actor;
}
void actors_handle_player_collision(void) BANKED REENTRANT;

// moved to vm_actor.c
//UWORD check_collision_in_direction(UWORD start_x, UWORD start_y, bounding_box_t *bounds, UWORD end_pos, col_check_dir_e check_dir) BANKED REENTRANT;

void activate_actors_in_row(UBYTE x, UBYTE y) BANKED REENTRANT;
void activate_actors_in_col(UBYTE x, UBYTE y) BANKED REENTRANT;
void player_init(void) BANKED REENTRANT;

// gbdk-nes: Make activate_actor_inline for speed
inline void activate_actor_inline(actor_t *actor) {
#ifdef STRICT
    // Check exists in inactive list
    UBYTE found = 0;
    actor_t *current = actors_inactive_head;
    DL_CONTAINS(current, actor, found);
    if (!found)
    {
        BGB_MESSAGE("Activated non inactive actor\n");
        __HandleCrash();
        return;
    }
#endif
    if (actor->active || actor->disabled) return;
    actor->active = TRUE;
    actor_set_anim_idle(actor);
    DL_REMOVE_ITEM(actors_inactive_head, actor);
    DL_PUSH_HEAD(actors_active_head, actor);
    actor->hscript_update = SCRIPT_TERMINATED;
    if (actor->script_update.bank) {
        script_execute(actor->script_update.bank, actor->script_update.ptr, &(actor->hscript_update), 0);
    }
    actor->hscript_hit = SCRIPT_TERMINATED;
}

inline void activate_actors_in_col_inline(UBYTE x, UBYTE y) {
    static actor_t *actor;
    actor = actors_inactive_head;
    while (actor) {
        UBYTE tx_left   = actor->pos.x >> 7;
        UBYTE ty_bottom = actor->pos.y >> 7;
        UBYTE tx_right  = ((actor->pos.x >> 4) + (actor->bounds.right)) >> 3;
        UBYTE ty_top    = ((actor->pos.y >> 4) + (actor->bounds.top)) >> 3;
        if (tx_left <= x && tx_right >= x && ty_top <= (y + SCREEN_TILE_REFRES_H) && ty_bottom >= y) {
            actor_t * next = actor->next;
            activate_actor_inline(actor);
            actor=next;
            continue;
        }
        actor = actor->next;
    }
}

void actor_update_bounds_x16(actor_t *actor) NONBANKED;
#endif
