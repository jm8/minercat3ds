#include <3ds.h>

#include "play.h"

int main() {
    gameInit();

    while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break;

        gameUpdate();
        gameRender();
    }

    gameExit();

    return 0;
}
