#pragma once

#include <stdbool.h>

typedef struct BlockCoords{
  int x;
  int y;
  int z;
} BlockCoords;

typedef struct Block{
  int id;
  int damage;
} Block;

void gameInit();
void gameUpdate();
void gameRender();
void gameExit();

void blockSetDamage(BlockCoords coords, int damage);
Block blockGet(BlockCoords coords);
bool selectedGet(BlockCoords *outCoords);

void cameraRotate(float right, float up);
void cameraMove(float forward, float up, float strafe);
