#ifdef SWITCH

#include <switch.h>

#include "tetris.h"

int running = 1;

int main() {
    gfxInitDefault();
    tetrisInit();

    while(appletMainLoop() && running) {
		hidScanInput();
		update();

        render();

		gfxFlushBuffers();
		gfxSwapBuffers();
		gfxWaitForVsync();
    }

    gfxExit();
    return 0;
}

#endif