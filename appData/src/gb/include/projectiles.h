#ifndef PROJECTILES_H
#define PROJECTILES_H

#include <gbdk/platform.h>

#include "math.h"
#include "collision.h"
#include "gbs_types.h"

#define MAX_PROJECTILES 5
#define MAX_PROJECTILE_DEFS 5

extern projectile_def_t projectile_defs[MAX_PROJECTILES];

void projectiles_init(void) BANKED;
void projectiles_update(void) BANKED;
void projectiles_render(void) NONBANKED;

#define PROJECTILE_ANIM_NOLOOP 0x01
#define PROJECTILE_STRONG 0x02

void projectile_launch(UBYTE index, upoint16_t *pos, UBYTE angle, UBYTE flags) BANKED;

inline void projectile_def_update_bounds_x16(projectile_def_t * projectile_def)
{
    projectile_def->bounds_x16.left = projectile_def->bounds.left << 4;
    projectile_def->bounds_x16.right = projectile_def->bounds.right << 4;
    projectile_def->bounds_x16.top = projectile_def->bounds.top << 4;
    projectile_def->bounds_x16.bottom = projectile_def->bounds.bottom << 4;
}

#endif
