#include <3ds.h>
#include <citro3d.h>
#include <stdbool.h>
#include <tex3ds.h>

#include "3ds/services/hid.h"
#include "c3d/types.h"
#include "play.h"

int main() {
    gameInit();

    bool lastKeyDownA = false;
    while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break;

        gameUpdate();

        if (kDown & KEY_A) {
            BlockCoords selected;
            if (!lastKeyDownA && selectedGet(&selected)) {
                blockSetDamage(selected, blockGet(selected).damage + 1);
            }
            lastKeyDownA = true;
        } else {
            lastKeyDownA = false;
        }

        gameRender();
    }

    gameExit();

    return 0;
}
