#include "3ds/console.h"
#include "3ds/gfx.h"
#include "game.h"
#include <stdio.h>
void debugInit() {
    consoleInit(GFX_BOTTOM, NULL);
}

void debugPrints(Game *g) {
    printf("\x1b[2J\x1b[H");
    printf("x: %.2f y: %.2f z: %.2f\n", g->playerController.pos.x, g->playerController.pos.y, g->playerController.pos.z);
    printf("isOnGround: %d\n", g->playerController.isOnGround);
}
