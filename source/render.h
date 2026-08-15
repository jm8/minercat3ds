#pragma once
#include "3ds/types.h"

typedef struct {
    float position[3];
    float texcoord[2];
    float normal[3];
} vertex;

typedef struct {
    int nVertex;
    int nSolidVertex;
    int nDamageVertex;
    vertex *vertices;
} Mesh;

enum FACE { FACE_PZ, FACE_MZ, FACE_PX, FACE_MX, FACE_PY, FACE_MY };

void meshInit(Mesh *mesh);

typedef struct Game Game;

void meshUpdateSelected(Mesh *m, u32 *chunk, int selected, int x, int yy, int z);
void meshBuild(Mesh *m, u32 *chunk);
void meshUpdateDamage(Mesh *m, u32 *chunk, int x, int yy, int z, int oldDamage, int damage);

void renderInit(Game *g);
void renderFrame(Game *g);
void renderExit(Game *g);
