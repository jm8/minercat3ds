#include <3ds.h>
#include <citro3d.h>
#include <stdbool.h>
#include <tex3ds.h>

#include "3ds/console.h"
#include "3ds/services/hid.h"
#include "3ds/services/y2r.h"
#include "c3d/maths.h"
#include "c3d/types.h"
#include "game.h"
#include "render.h"

char *world;

void cameraUpdate(Game *g) {
    circlePosition circle;
    hidCircleRead(&circle);
    float moveX = circle.dx / 156.0f;
    float moveY = circle.dy / 156.0f;

    u32 held = hidKeysHeld();

    if (held & KEY_DLEFT)
        g->camera.yaw += 0.03f;

    if (held & KEY_DRIGHT)
        g->camera.yaw -= 0.03f;

    if (held & KEY_DUP)
        g->camera.pitch += 0.03f;

    if (held & KEY_DDOWN)
        g->camera.pitch -= 0.03f;

    printf("Pitch: %f Yaw: %f\n", g->camera.pitch, g->camera.yaw);

    float cp = cosf(g->camera.pitch);
    float sp = sinf(g->camera.pitch);

    float cy = cosf(-g->camera.yaw);
    float sy = sinf(-g->camera.yaw);

    float forwardX = sy * cp;
    float forwardY = sp;
    float forwardZ = -cy * cp;

    float speed  = 0.1;
    float rightX = cy;
    float rightZ = sy;

    g->camera.x += (forwardX * moveY + rightX * moveX) * speed;
    g->camera.y += (forwardY * moveY) * speed;
    g->camera.z += (forwardZ * moveY + rightZ * moveX) * speed;

    if (held & KEY_L)
        g->camera.y -= speed;
    if (held & KEY_R)
        g->camera.y += speed;
}

static Game g;

int getblock(Game *g, int x, int y, int z) {
    if (x < 0 || x >= WORLD_WIDTH)
        return 0;
    if (z < 0 || z >= WORLD_WIDTH)
        return 0;
    if (y < 0 || y >= CHUNK_HEIGHT * NUM_CHUNKS)
        return 0;

    return g->world[W(x, y, z)] & MASK_BLOCK;
}

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

    g->selected = false;
    while (d < max_distance) {
        d += step;
        p = FVec3_Add(p, dp);

        int x     = p.x;
        int y     = p.y;
        int z     = p.z;
        int block = getblock(g, x, y, z);
        if (block) {
            g->selected  = true;
            g->selectedX = x;
            g->selectedY = y;
            g->selectedZ = z;
            break;
        }
    }
}

void damage(Game *g) {
    int i         = W(g->selectedX, g->selectedY, g->selectedZ);
    int oldDamage = (g->world[i] & MASK_DAMAGE) >> SHIFT_DAMAGE;
    int newDamage = oldDamage + 1;
    g->world[i]   = (g->world[i] & MASK_BLOCK) | (newDamage << SHIFT_DAMAGE);
    meshUpdateDamage(&g->mesh, g->world, g->selectedX, g->selectedY, g->selectedZ, oldDamage, newDamage);
}

int main() {
    renderInit(&g);
    // g.camera.x     = 8.7;
    // g.camera.y     = 21.6;
    // g.camera.z     = -7;
    // g.camera.pitch = -0.7;
    // g.camera.yaw   = -3.14;
    g.camera.x     = 8.7;
    g.camera.y     = 7.7;
    g.camera.z     = -1.5;
    g.camera.pitch = -1.749;
    g.camera.yaw   = -3.14;

    g.world = linearAlloc(NUM_CHUNKS * CHUNK_HEIGHT * WORLD_WIDTH * WORLD_WIDTH);
    for (int y = 0; y < NUM_CHUNKS * CHUNK_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            for (int z = 0; z < WORLD_WIDTH; z++) {
                g.world[W(x, y, z)] = (y < 8) ? 4 : (y == 15) ? 2 : 3;
            }
        }
    }
    meshBuild(&g.mesh, g.world);
    consoleInit(GFX_BOTTOM, NULL);
    bool lastKeyDownA = false;
    while (aptMainLoop()) {
        printf("\x1b[2J\x1b[H");

        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break;

        cameraUpdate(&g);

        raycast(&g);
        meshUpdateSelected(&g.mesh, g.world, g.selected, g.selectedX, g.selectedY, g.selectedZ);

        if (kDown & KEY_A) {
            if (!lastKeyDownA && g.selected) {
                damage(&g);
            }
            lastKeyDownA = true;
        } else {
            lastKeyDownA = false;
        }

        printf("raycast (%d, %.1d, %.1d, %.1d)\n", g.selected, g.selectedX, g.selectedY, g.selectedZ);
        printf("Camera (%.1f, %.1f, %.1f)\n", g.camera.x, g.camera.y, g.camera.z);

        renderFrame(&g);
    }

    renderExit(&g);

    return 0;
}
