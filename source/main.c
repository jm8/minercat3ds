#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>

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

int main() {
    renderInit(&g);
    g.camera.z += 3;
    g.camera.y += 1;

    g.world = linearAlloc(NUM_CHUKS * CHUNK_HEIGHT * WORLD_WIDTH * WORLD_WIDTH);
    for (int y = 0; y < NUM_CHUKS * CHUNK_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            for (int z = 0; z < WORLD_WIDTH; z++) {
                g.world[W(x, y, z)] = 1;
            }
        }
    }
    meshBuild(&g.mesh, g.world);

    // Main loop
    while (aptMainLoop()) {
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu

        cameraUpdate(&g);
        // Render the scene
        renderFrame(&g);
    }

    // Deinitialize the scene
    renderExit(&g);

    return 0;
}
