#include <3ds.h>
#include <math.h>
#include "3ds/services/hid.h"
#include "c3d/maths.h"
#include "c3d/types.h"
#include "controller.h"
#include "play.h"

#define GRAVITY 0.008
#define TERMINAL_VELOCITY 0.8
#define JUMP_VEL 0.2

#define NUM_CORNERS 12
#define EYE_HEIGHT 1.8f
#define PLAYER_WIDTH 0.3f
static float corners[NUM_CORNERS][3] = {
    {-PLAYER_WIDTH, -EYE_HEIGHT, -PLAYER_WIDTH},
    {-PLAYER_WIDTH, -EYE_HEIGHT, PLAYER_WIDTH},
    {PLAYER_WIDTH, -EYE_HEIGHT, -PLAYER_WIDTH},
    {PLAYER_WIDTH, -EYE_HEIGHT, PLAYER_WIDTH},
    {-PLAYER_WIDTH, 0, -PLAYER_WIDTH},
    {-PLAYER_WIDTH, 0, PLAYER_WIDTH},
    {PLAYER_WIDTH, 0, -PLAYER_WIDTH},
    {PLAYER_WIDTH, 0, PLAYER_WIDTH},
    {-PLAYER_WIDTH, (0 - EYE_HEIGHT) / 2, -PLAYER_WIDTH},
    {-PLAYER_WIDTH, (0 - EYE_HEIGHT) / 2, PLAYER_WIDTH},
    {PLAYER_WIDTH, (0 - EYE_HEIGHT) / 2, -PLAYER_WIDTH},
    {PLAYER_WIDTH, (0 - EYE_HEIGHT) / 2, PLAYER_WIDTH},
};

bool fly    = false;
bool noclip = false;
bool fast   = false;

bool isColliding(PlayerController *p) {
    if (noclip)
        return false;
    for (int i = 0; i < NUM_CORNERS; i++) {
        if (blockGet((BlockCoords){floorf(p->pos.x + corners[i][0]), floorf(p->pos.y + corners[i][1]), floorf(p->pos.z + corners[i][2])}, 1).id)
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
    float speed = fast ? 0.5 : 0.12;

    u32 down = hidKeysDown();
    u32 held = hidKeysHeld();
    circlePosition circle;
    circleRead(&circle);
    float moveX = circle.dx / 156.0f;
    if (fabsf(moveX) < 0.1)
        moveX = 0;
    float moveY = circle.dy / 156.0f;
    if (fabsf(moveY) < 0.1)
        moveY = 0;

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

    if (fly) {
        p->vel.y = 0;
        if (held & KEY_L) {
            p->vel.y = -speed;
        }
        if (held & KEY_R) {
            p->vel.y = speed;
        }
    }

    if (!fly) {
        p->vel.y -= GRAVITY;
        if (p->vel.y < -TERMINAL_VELOCITY)
            p->vel.y = -TERMINAL_VELOCITY;
    }

    if (moveAxis(p, FVec3_New(0, p->vel.y, 0))) {
        p->isOnGround = (p->vel.y < 0);
        p->vel.y      = 0;
    } else {
        p->isOnGround = false;
    }

    moveAxis(p, FVec3_New(p->vel.x, 0, 0));
    moveAxis(p, FVec3_New(0, 0, p->vel.z));

    if (!fly && (held & (KEY_A | KEY_L)) && p->isOnGround) {
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

    touchPosition touchPos;
    hidTouchRead(&touchPos);

    float touchSpeed = 6.28f / 320;
    if ((down & KEY_TOUCH)) {
        p->touchStart      = touchPos;
        p->touchStartPitch = p->pitch;
        p->touchStartYaw   = p->yaw;
    } else if ((held & KEY_TOUCH)) {
        float targetPitch = p->touchStartPitch - touchSpeed * (touchPos.py - p->touchStart.py);
        float targetYaw   = p->touchStartYaw - touchSpeed * (touchPos.px - p->touchStart.px);
        p->pitch          = (p->pitch + targetPitch) / 2.0;
        p->yaw            = (p->yaw + targetYaw) / 2.0;
    }

    if (p->pitch < -1.55) {
        p->pitch = -1.55;
    }
    if (p->pitch > 1.55) {
        p->pitch = 1.55;
    }

    BlockCoords selected;
    if ((held & (KEY_B | KEY_TOUCH)) && selectedGet(&selected)) {
        blockSetDamage(selected, blockGet(selected, 0).damage + 1);
    }
}
