#pragma once

#include <3ds.h>
#include <citro3d.h>
#include "controller.h"
#include "play.h"
#include <stdbool.h>
#include <tex3ds.h>

#include "render.h"

#define WORLD_WIDTH 24
#define CHUNK_HEIGHT 16
// #define NUM_CHUNKS 516
#define NUM_CHUNKS 1
#define WORLD_HEIGHT (NUM_CHUNKS * CHUNK_HEIGHT)
#define WORLD_SIZE (WORLD_WIDTH * WORLD_WIDTH * CHUNK_HEIGHT * NUM_CHUNKS)

#define MASK_BLOCK 0x3f
#define SHIFT_DAMAGE 6

typedef struct Camera {
    float x, y, z;
    float yaw;
    float pitch;
} Camera;

typedef struct Game {
    Camera camera;
    Mesh mesh;
    PlayerController playerController;

    C3D_RenderTarget *leftTarget;
    C3D_RenderTarget *rightTarget;

    u32 *world;

    bool hasSelected;
    BlockCoords selected;
} Game;

#define W(x, y, z) ((y) * WORLD_WIDTH * WORLD_WIDTH + (x) * WORLD_WIDTH + (z))

void raycast(Game *g);
