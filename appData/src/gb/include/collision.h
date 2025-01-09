#ifndef COLLISIONS_H
#define COLLISIONS_H

#include <gbdk/platform.h>

#include "math.h"
#include "bankdata.h"

#define COLLISION_TOP 0x1
#define COLLISION_BOTTOM 0x2
#define COLLISION_LEFT 0x4
#define COLLISION_RIGHT 0x8
#define COLLISION_ALL 0xF
#define TILE_PROP_LADDER 0x10

typedef struct bounding_box_t {
    BYTE left, right, top, bottom;
} bounding_box_t;

typedef struct bounding_box_16_t {
    WORD left, right, top, bottom;
} bounding_box_16_t;

extern UBYTE collision_bank;
extern unsigned char *collision_ptr;
extern UBYTE image_tile_width;
extern UBYTE image_tile_height;

/**
 * Check if point is within positioned bounding box.
 *
 * @param bb Pointer to bounding box
 * @param offset Pointer to position offset for bounding box (e.g Actor position)
 * @param point Pointer to position to look for within bounding box
 * @return Point is within bounding box
 */
inline UBYTE bb_contains(bounding_box_t *bb, upoint16_t *offset, upoint16_t *point) {
    if ((point->x < (offset->x >> 4) + bb->left) || 
        (point->x > (offset->x >> 4) + bb->right)) return FALSE;
    if ((point->y < (offset->y >> 4) + bb->top) || 
        (point->y > (offset->y >> 4) + bb->bottom)) return FALSE;
    return TRUE;
}

/**
 * Check if two positioned bounding boxes intersect.
 *
 * @param bb_a Pointer to bounding box A
 * @param offset_a Pointer to position offset for bounding box A
 * @param bb_b Pointer to bounding box B
 * @param offset_b Pointer to position offset for bounding box B
 * @return Positioned bounding boxes intersect
 */
inline UBYTE bb_intersects(bounding_box_16_t *bb_x16_a, upoint16_t *offset_a, bounding_box_16_t *bb_x16_b, upoint16_t *offset_b) {
    if ((offset_b->x + bb_x16_b->left   > offset_a->x + bb_x16_a->right) ||
        (offset_b->x + bb_x16_b->right  < offset_a->x + bb_x16_a->left)) return FALSE;
    if ((offset_b->y + bb_x16_b->top    > offset_a->y + bb_x16_a->bottom) ||
        (offset_b->y + bb_x16_b->bottom < offset_a->y + bb_x16_a->top)) return FALSE;
    return TRUE;
}

/**
 * Return collision tile value at given tile x,y coordinate.
 *
 * @param tx Left tile
 * @param ty Top tile
 * @return Tile value, 0 if no collisions, COLLISION_ALL if out of bounds
 */
UBYTE tile_at(UBYTE tx, UBYTE ty) __no_overlay_locals;

#endif
