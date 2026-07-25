#pragma once

typedef struct {
    float position[3];
    float texcoord[2];
    float normal[3];
} vertex;

typedef struct {
  int nVertex;
  vertex *vertices;
} Mesh;

enum FACE { FACE_PZ, FACE_MZ, FACE_PX, FACE_MX, FACE_PY, FACE_MY };

void meshInit(Mesh *mesh);

void meshAddFace(Mesh *mesh, int face, int x, int y, int z, int texX, int texY, int overlay);
void meshBuild(Mesh *m, char *chunk);

typedef struct Game Game;

void renderInit(Game *g);
void renderFrame(Game *g);
void renderExit(Game *g);
