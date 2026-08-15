#pragma once

#include "3ds/services/hid.h"
#include "3ds/types.h"
#include "c3d/types.h"

typedef struct PlayerController {
    C3D_FVec pos;
    C3D_FVec vel;
    float pitch;
    float yaw;
    bool isOnGround;
    float touchStartPitch;
    float touchStartYaw;
    touchPosition touchStart;
} PlayerController;

void playerController(PlayerController *p);
