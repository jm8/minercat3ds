#include <3ds.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    gfxInitDefault();

    consoleInit(GFX_TOP, NULL);

    printf("Hello, Nintendo 3DS!\n");
    printf("Press START to exit.\n");

    while (aptMainLoop())
    {
        hidScanInput();

        u32 kDown = hidKeysDown();

        if (kDown & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();

    return 0;
}
