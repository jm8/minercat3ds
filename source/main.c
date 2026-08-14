#include <3ds.h>
#include <citro3d.h>
#include <stdbool.h>
#include <tex3ds.h>

#include "3ds/services/hid.h"
#include "c3d/types.h"
#include "play.h"

void cameraUpdate() {
    circlePosition circle;
    hidCircleRead(&circle);
    float moveX = circle.dx / 156.0f;
    float moveY = circle.dy / 156.0f;

    u32 held = hidKeysHeld();

    if (held & KEY_DLEFT)
        cameraRotate(0.03, 0);

    if (held & KEY_DRIGHT)
        cameraRotate(-0.03, 0);

    if (held & KEY_DUP)
        cameraRotate(0, 0.03);

    if (held & KEY_DDOWN)
        cameraRotate(0, -0.03);

    float speed = 0.1;

    cameraMove(moveY * speed, 0, moveX * speed);
    if (held & KEY_L)
        cameraMove(0, -speed, 0);
    if (held & KEY_R)
        cameraMove(0, speed, 0);
}

int main() {
    gameInit();

    bool lastKeyDownA = false;
    while (aptMainLoop()) {
        printf("\x1b[2J\x1b[H");

        hidScanInput();

        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break;

        cameraUpdate();

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
