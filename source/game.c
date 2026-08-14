#include "game.h"
#include "play.h"

void raycast(Game *g) {
    float max_distance = 30;
    float step         = 0.1;

    C3D_FVec p = FVec3_New(g->camera.x, g->camera.y, g->camera.z);

    float cp = cosf(g->camera.pitch);
    float sp = sinf(g->camera.pitch);

    float cy = cosf(-g->camera.yaw);
    float sy = sinf(-g->camera.yaw);

    float forwardX = sy * cp;
    float forwardY = sp;
    float forwardZ = -cy * cp;

    C3D_FVec dp = FVec3_New(step * forwardX, step * forwardY, step * forwardZ);

    float d = 0;

    g->hasSelected = false;
    while (d < max_distance) {
        d += step;
        p = FVec3_Add(p, dp);

        BlockCoords b = {p.x, p.y, p.z};
        int block     = blockGet(b).id;
        if (block) {
            g->hasSelected = true;
            g->selected    = b;
            break;
        }
    }
}
