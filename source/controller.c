#include "3ds/services/cdcchk.h"
#include "3ds/services/hid.h"
#include "c3d/maths.h"
#include "c3d/types.h"
#include "controller.h"
#include "game.h"

#define GRAVITY 0.008
#define TERMINAL_VELOCITY 2
#define JUMP_VEL 0.2

#define NUM_CORNERS 1
static float corners[NUM_CORNERS][3] = {{0, -1.8, 0}};

bool isColliding(PlayerController *p) {
    for (int i = 0; i < NUM_CORNERS; i++) {
        if (blockGet((BlockCoords){p->pos.x + corners[i][0], p->pos.y + corners[i][1], p->pos.z + corners[i][2]}).id)
            return true;
    }
    return false;
}

bool moveAxis(PlayerController *p, C3D_FVec vel) {
    int steps     = 10;
    C3D_FVec step = FVec3_Scale(vel, 1.0f / steps);
    for (int i = 0; i < steps; i++) {
        p->pos = FVec3_Add(p->pos, step);
        if (isColliding(p)) {
            p->pos = FVec3_Subtract(p->pos, step);
            return true;
        }
    }
    return false;
}

void playerController(PlayerController *p) {
    p->vel.y -= GRAVITY;
    if (p->vel.y < -TERMINAL_VELOCITY)
        p->vel.y = -TERMINAL_VELOCITY;

    if (moveAxis(p, FVec3_New(0, p->vel.y, 0))) {
        p->isOnGround = (p->vel.y < 0);
        p->vel.y      = 0;
    } else {
        p->isOnGround = false;
    }

    u32 down = hidKeysDown();
    u32 held = hidKeysHeld();
    if ((held & KEY_A) && p->isOnGround) {
        p->vel.y = JUMP_VEL;
    }
}
