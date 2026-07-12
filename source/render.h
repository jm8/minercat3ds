#pragma once

typedef struct {
    float position[3];
    float texcoord[2];
    float normal[3];
} vertex;

typedef struct {
  int nVertex;
  vertex *vertices;
} ChunkMesh;

enum FACE { FACE_PZ, FACE_MZ, FACE_PX, FACE_MX, FACE_PY, FACE_MY };

void chunkMeshInit(ChunkMesh *mesh);

void addFace(ChunkMesh *mesh, int face, int x, int y, int block);

typedef struct Game Game;

void renderInit(Game *g);
void renderFrame(Game *g);
void renderExit(Game *g);
