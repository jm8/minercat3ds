#include <3ds.h>
#include "c3d/maths.h"
#include "c3d/types.h"
#include "controller.h"
#include "play.h"

#define GRAVITY 0.008
#define TERMINAL_VELOCITY 0.8
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
    float speed = 0.12;

    u32 down = hidKeysDown();
    u32 held = hidKeysHeld();
    circlePosition circle;
    circleRead(&circle);
    float moveX = circle.dx / 156.0f;
    float moveY = circle.dy / 156.0f;

    float cp = cosf(p->pitch);
    float sp = sinf(p->pitch);

    float cy = cosf(-p->yaw);
    float sy = sinf(-p->yaw);

    // float forwardX = sy * cp;
    // float forwardY = sp;
    // float forwardZ = -cy * cp;
    float forwardX = sy;
    float forwardZ = -cy;

    float rightX = cy;
    float rightZ = sy;

    p->vel.x = forwardX * speed * moveY + rightX * speed * moveX;
    p->vel.z = forwardZ * speed * moveY + rightZ * speed * moveX;

    p->vel.y -= GRAVITY;
    if (p->vel.y < -TERMINAL_VELOCITY)
        p->vel.y = -TERMINAL_VELOCITY;

    if (moveAxis(p, FVec3_New(0, p->vel.y, 0))) {
        p->isOnGround = (p->vel.y < 0);
        p->vel.y      = 0;
    } else {
        p->isOnGround = false;
    }

    moveAxis(p, FVec3_New(p->vel.x, 0, 0));
    moveAxis(p, FVec3_New(0, 0, p->vel.z));

    if ((held & KEY_A) && p->isOnGround) {
        p->vel.y = JUMP_VEL;
    }

    float turnSpeed = 0.06;
    if ((held & KEY_DUP)) {
        p->pitch += turnSpeed;
    }
    if ((held & KEY_DDOWN)) {
        p->pitch -= turnSpeed;
    }
    if ((held & KEY_DRIGHT)) {
        p->yaw -= turnSpeed;
    }
    if ((held & KEY_DLEFT)) {
        p->yaw += turnSpeed;
    }
    if (p->pitch < -1.55) {
        p->pitch = -1.55;
    }
    if (p->pitch > 1.55) {
        p->pitch = 1.55;
    }

    BlockCoords selected;
    if ((held & KEY_B) && selectedGet(&selected)) {
        blockSetDamage(selected, blockGet(selected).damage + 1);
    }
}
