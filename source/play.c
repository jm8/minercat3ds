#include "play.h"
#include "game.h"
#include "render.h"
#include "controller.h"
#include "debug.h"

static Game g;

void gameInit() {
    renderInit(&g);

    g.playerController.pos.x = 8.7;
    g.playerController.pos.y = 37.7;
    g.playerController.pos.z = 8.7;
    g.playerController.pitch = -1;
    g.playerController.yaw   = -3.14;

    g.world = linearAlloc(NUM_CHUNKS * CHUNK_HEIGHT * WORLD_WIDTH * WORLD_WIDTH);
    for (int y = 0; y < NUM_CHUNKS * CHUNK_HEIGHT; y++) {
        for (int x = 0; x < WORLD_WIDTH; x++) {
            for (int z = 0; z < WORLD_WIDTH; z++) {
                g.world[W(x, y, z)] = 1;
            }
        }
    }

    meshBuild(&g.mesh, g.world);
    debugInit();
}

void gameUpdate() {
    playerController(&g.playerController);
    g.camera.x     = g.playerController.pos.x;
    g.camera.y     = g.playerController.pos.y;
    g.camera.z     = g.playerController.pos.z;
    g.camera.pitch = g.playerController.pitch;
    g.camera.yaw   = g.playerController.yaw;
    raycast(&g);
    meshUpdateSelected(&g.mesh, g.world, g.hasSelected, g.selected.x, g.selected.y, g.selected.z);
    debugPrints(&g);
}

void blockSetDamage(BlockCoords coords, int newDamage) {
    int i = W(coords.x, coords.y, coords.z);

    int oldDamage = (g.world[i] & MASK_DAMAGE) >> SHIFT_DAMAGE;
    if (newDamage > 10) {
        g.world[i] = 0;
        meshBuild(&g.mesh, g.world);
    } else {
        g.world[i] = (g.world[i] & MASK_BLOCK) | (newDamage << SHIFT_DAMAGE);
        meshUpdateDamage(&g.mesh, g.world, coords.x, coords.y, coords.z, oldDamage, newDamage);
    }
}

Block blockGet(BlockCoords coords) {
    int x = coords.x;
    int y = coords.y;
    int z = coords.z;
    Block block;
    block.damage = 0;
    block.id     = 0;
    if (x < 0 || x >= WORLD_WIDTH)
        return block;
    if (z < 0 || z >= WORLD_WIDTH)
        return block;
    if (y < 0 || y >= CHUNK_HEIGHT * NUM_CHUNKS)
        return block;

    int data     = g.world[W(x, y, z)];
    block.id     = (data & MASK_BLOCK);
    block.damage = (data & MASK_DAMAGE) >> SHIFT_DAMAGE;
    return block;
}

bool selectedGet(BlockCoords *outCoords) {
    if (g.hasSelected) {
        *outCoords = g.selected;
    }
    return g.hasSelected;
}

void gameRender() {
    renderFrame(&g);
}

void gameExit() {
    renderExit(&g);
}
void cameraRotate(float right, float up) {
    g.camera.pitch += up;
    g.camera.yaw += right;
}

void cameraMove(float forward, float up, float strafe) {
    float cp = cosf(g.camera.pitch);
    float sp = sinf(g.camera.pitch);

    float cy = cosf(-g.camera.yaw);
    float sy = sinf(-g.camera.yaw);

    float forwardX = sy * cp;
    float forwardY = sp;
    float forwardZ = -cy * cp;

    float rightX = cy;
    float rightZ = sy;

    g.camera.x += (forwardX * forward + rightX * strafe);
    g.camera.y += (forwardY * forward);
    g.camera.z += (forwardZ * forward + rightZ * strafe);

    g.camera.y += up;
}
