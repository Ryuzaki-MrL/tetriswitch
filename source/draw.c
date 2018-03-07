#ifdef SWITCH
    #include <switch.h>
#else
    #include "switchdefs.h"
#endif

#include "draw.h"

static u32* framebuf;
static u32 fbwidth;
static u32 fbheight;

void drawStart() {
    framebuf = (u32*)gfxGetFramebuffer(&fbwidth, &fbheight);
}

void drawEnd() {
    gfxFlushBuffers();
    gfxSwapBuffers();
    gfxWaitForVsync();
}

void drawPixel(int x, int y, u32 color) {
    if (x >= 0 && x < fbwidth && y >= 0 && y < fbheight) {
        int pos = y * fbwidth + x;
        framebuf[pos] = color;
    }
}

void drawLine(int x1, int y1, int x2, int y2, u32 color) {
    int x, y;
    if (x1 == x2) {
        if (y1 < y2) {
            for (y = y1; y <= y2; y++) drawPixel(x1, y, color);
        } else {
            for (y = y2; y <= y1; y++) drawPixel(x1, y, color);
        }
    } else {
        if (x1 < x2) {
            for (x = x1; x <= x2; x++) drawPixel(x, y1, color);
        } else {
            for (x = x2; x <= x1; x++) drawPixel(x, y1, color);
        }
    }
}

void drawRectangle(int x1, int y1, int x2, int y2, u32 color) {
    drawLine(x1, y1, x2, y1, color);
    drawLine(x2, y1, x2, y2, color);
    drawLine(x1, y2, x2, y2, color);
    drawLine(x1, y1, x1, y2, color);
}

void drawFillRect(int x1, int y1, int x2, int y2, u32 color) {
    int X1, X2, Y1, Y2, i, j;
    if (x1 < x2) {
        X1 = x1; X2 = x2;
    } else {
        X1 = x2; X2 = x1;
    }
    if (y1 < y2) {
        Y1 = y1; Y2 = y2;
    } else {
        Y1 = y2; Y2 = y1;
    }
    for (i = X1; i <= X2; i++) {
        for (j = Y1; j <= Y2; j++) {
            drawPixel(i, j, color);
        }
    }
}

void drawBitmap(int x, int y, Bitmap bmp) {
    int xx, yy;
    for (yy = y; yy < y + bmp.height; ++yy) {
        for (xx = x; xx < x + bmp.width; ++xx) {
            if (xx >= 0 && xx < fbwidth && yy >= 0 && yy < fbheight) {
                int pos = yy * fbwidth + xx;
                framebuf[pos] = bmp.buf[pos];
            }
        }
    }
}