#pragma once

#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>

#include "render.h"

#define WORLD_WIDTH 24
#define CHUNK_HEIGHT 16
#define NUM_CHUKS 516
#define WORLD_SIZE (WORLD_WIDTH * WORLD_WIDTH * CHUNK_HEIGHT * NUM_CHUNKS)

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
} Game;

#define W(x,y,z) ((y) * WORLD_WIDTH * WORLD_WIDTH + (x) * WORLD_WIDTH + (z) * WORLD_WIDTH)


