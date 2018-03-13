#include <stdio.h>
#include <stdarg.h>
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

void drawClearScreen(u32 color) {
	int i;
    for (i = 0; i < fbwidth * fbheight; ++i) {
        framebuf[i] = color;
    }
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
        for (y = y1; y <= y2; ++y) {
            drawPixel(x1, y, color);
        }
    } else {
        for (x = x1; x <= x2; ++x) {
            drawPixel(x, y1, color);
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
    int i, j;
    for (i = x1; i <= x2; ++i) {
        for (j = y1; j <= y2; ++j) {
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

void drawText(const ffnt_header_t* font, int x, int y, u32 color, const char* str) {
    color_t clr;
    clr.abgr = color;
    DrawText(font, x, y, clr, str);
}

void drawTextFormat(const ffnt_header_t* font, int x, int y, u32 color, const char* str, ...) {
    char buffer[256];
    va_list valist;
    va_start(valist, str);
    vsnprintf(buffer, 255, str, valist);
    drawText(font, x, y, color, buffer);
    va_end(valist);
}
