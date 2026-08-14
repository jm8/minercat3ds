#pragma once

#include "c3d/types.h"

typedef struct PlayerController {
    C3D_FVec pos;
    C3D_FVec vel;
    float pitch;
    float yaw;
    bool isOnGround;
} PlayerController;

void playerController(PlayerController *p);
