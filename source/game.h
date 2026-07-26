#pragma once

#include <3ds.h>
#include <citro3d.h>
#include <stdbool.h>
#include <tex3ds.h>

#include "render.h"

#define WORLD_WIDTH 24
#define CHUNK_HEIGHT 16
// #define NUM_CHUNKS 516
#define NUM_CHUNKS 1
#define WORLD_SIZE (WORLD_WIDTH * WORLD_WIDTH * CHUNK_HEIGHT * NUM_CHUNKS)

#define MASK_BLOCK 0x7
#define MASK_DAMAGE 0x1f8
#define SHIFT_DAMAGE 3

typedef struct Camera {
    float x, y, z;
    float yaw;
    float pitch;
} Camera;

typedef struct Game {
    Camera camera;
    Mesh mesh;

    C3D_RenderTarget *target;

    char *world;

    bool selected;
    int selectedX;
    int selectedY;
    int selectedZ;
} Game;

#define W(x,y,z) ((y) * WORLD_WIDTH * WORLD_WIDTH + (x) * WORLD_WIDTH + (z) * WORLD_WIDTH)


