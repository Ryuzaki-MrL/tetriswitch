#ifdef SWITCH

#include <string.h>
#include <stdio.h>
#include <switch.h>

#include "tetris.h"

#define TICKS_PER_SECOND 60
#define SKIP_TICKS (1000 / TICKS_PER_SECOND)
#define MAX_FRAMESKIP 5

int running = 1;

int main() {
    gfxInitDefault();
    tetrisInit();

    u64 next;
    timeGetCurrentTime(TimeType_Default, &next);

    while(appletMainLoop() && running) {
        int loops = 0;
        u64 current;
        timeGetCurrentTime(TimeType_Default, &current);

        while(current > next && loops < MAX_FRAMESKIP) {
            hidScanInput();
            update();
            next += SKIP_TICKS;
            ++loops;
        }

        render();
    }

    gfxExit();
    return 0;
}

#endif