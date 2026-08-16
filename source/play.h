#pragma once

#include "3ds/types.h"
#include <stdbool.h>

typedef struct BlockCoords {
    int x;
    int y;
    int z;
} BlockCoords;

typedef struct Block {
    int id;
    int durability;
    int damage;
} Block;

void gameInit();
void gameUpdate();
void gameRender();
void gameExit();

void blockSetDamage(BlockCoords coords, int damage);
Block blockGet(BlockCoords coords, int def);
bool selectedGet(BlockCoords *outCoords);
int visibleDamage(u32 block);
